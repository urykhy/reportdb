
#ifndef _TBB_TRAMPOLINE_H__
#define _TBB_TRAMPOLINE_H__

#include <task_scheduler_init.h>
#include <blocked_range.h>
#include <parallel_for.h>
#include <pipeline.h>
#include <list>
#include <Log4.hpp>
#include <memory>

namespace Util {

    template<class D, class T>
    void parallel(const D& data, T& t)
    {
        typedef tbb::blocked_range<typename D::size_type> Range;
        tbb::parallel_for(
                          Range(0, data.size()),
                          [&t, &data](auto r){
                              for (auto i = r.begin(); i!= r.end(); i++)
                                  t(data[i]);
                          }
        );
    }

    namespace internal {

        template<class D, class T>
        struct Ticket {
            typedef typename D::value_type value_type;
            typedef typename T::TmpResult TmpResult;
            typedef std::pair<value_type, TmpResult> Tick;

            typedef typename D::const_iterator Iterator;

            const D& data;
            Iterator iter;
            T& t;

            typedef std::list<Tick> DataFlowT;
            DataFlowT active_data;

            Ticket(const D& dataIn, T& tIn)
            : data(dataIn),
            iter(dataIn.begin()),
            t(tIn)
            {
            }

            void* emit(void*)
            {
                if (iter != data.end()) {
                    active_data.push_back(Tick(*iter, TmpResult()));
                    LOG4_DEBUG("open " << *iter);
                    ++iter;
                    return &active_data.back();
                } else {
                    return NULL;
                }
            }
            void* work(void* item) // parallel
            {
                Tick * tick_item = static_cast<Tick *>(item);
                LOG4_DEBUG("worker start " << tick_item->first);
                t.work(tick_item->first, tick_item->second);
                LOG4_DEBUG("worker done " << tick_item->first);
                return item;
            }
            void* join(void* item)
            {
                Tick * tick_item = static_cast<Tick *>(item);
                LOG4_DEBUG("join start " << tick_item->first);
                t.join(tick_item->first, tick_item->second);
                LOG4_DEBUG("join done " << tick_item->first);

                if (&active_data.front() == tick_item)
                {
                    active_data.pop_front();
                } else {
                    LOG4_ERROR("OUT-OF-ORDER data flow though pipeline!");
                }

                return NULL;
            }
        };

        template<bool serial_flag, class F>
        struct Stage : public tbb::filter {
            F func;
            Stage(F f) : tbb::filter(serial_flag), func(f) {}
            virtual ~Stage() throw() {}
            virtual void* operator()(void* item) { return func(item); }
        };

        template<bool serial_flag, class L, class F>
        auto add_stage(L& list, F f)
        {
            return list.push_back(std::make_shared<Stage<serial_flag, F>>(f));
        }

    } // namespace internal

    template<class D, class T>
    void pipeline(const D& data, T& t, int thr_count = -1)
    {
        typedef internal::Ticket<D, T> Ticket;
        Ticket ticket(data, t);

        using filter_var = std::shared_ptr<tbb::filter>;
        std::list<filter_var> filter_list;

        internal::add_stage<true> (filter_list, [&ticket](void* item){ return ticket.emit(item); });
        internal::add_stage<false>(filter_list, [&ticket](void* item){ return ticket.work(item); });
        internal::add_stage<true> (filter_list, [&ticket](void* item){ return ticket.join(item); });

        tbb::pipeline pl;
        for (auto x : filter_list) {pl.add_filter(*x.get());}

        pl.run(thr_count > 0 ? thr_count : data.size());
        pl.clear();
    }


} // namespace Util

#endif /* _TBB_TRAMPOLINE_H__ */

