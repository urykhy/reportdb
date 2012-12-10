
#ifndef _TBB_TRAMPOLINE_H__
#define _TBB_TRAMPOLINE_H__

#include <task_scheduler_init.h>
#include <blocked_range.h>
#include <parallel_for.h>
#include <pipeline.h>
#include <list>
#include <Log4.hpp>

namespace Util {

	namespace internal {
	template<class D, class T>
		struct ApplyT
		{
			const D& data;
			T& t;

			ApplyT(const D& dataIn, T& tIn) : data(dataIn), t(tIn)
			{
				;;
			}
			template<class R>
			void operator()(const R& r) const
			{
				for (typename D::size_type i = r.begin(); i!= r.end(); ++i)
				{
					t(data[i]);
				}
			}
		};
	} // namespace internal

	template<class D, class T>
	void parallel(const D& data, T& t)
	{
		typedef tbb::blocked_range<typename D::size_type> Range;
		tbb::parallel_for(
				Range(0, data.size()),
				internal::ApplyT<D, T>(data, t)
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

		template<bool serial_flag, class T, void* (T::*callback)(void*)>
		struct Stage : public tbb::filter {
			T& ticket;
			Stage(T& ticketIn) : tbb::filter(serial_flag), ticket(ticketIn) {}
			virtual ~Stage() throw() {}
			virtual void* operator()(void* item)
			{
				return ((&ticket)->*callback)(item);
			}
		};
	} // namespace internal

	template<class D, class T>
	void pipeline(const D& data, T& t)
	{
		typedef internal::Ticket<D, T> Ticket;
		Ticket ticket(data, t);

		tbb::pipeline pl;
		internal::Stage<true,  Ticket, &Ticket::emit> s1(ticket);
		internal::Stage<false, Ticket, &Ticket::work> s2(ticket);
		internal::Stage<true,  Ticket, &Ticket::join> s3(ticket);

		pl.add_filter(s1);
		pl.add_filter(s2);
		pl.add_filter(s3);
		pl.run(data.size());
		pl.clear();
		// FIXME run() we must pass N of threads + 1
	}


} // namespace Util

#endif /* _TBB_TRAMPOLINE_H__ */

