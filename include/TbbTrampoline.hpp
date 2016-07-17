
#ifndef _TBB_TRAMPOLINE_H__
#define _TBB_TRAMPOLINE_H__

#include <list>
#include <Log4.hpp>
#include <memory>
#include <future>

namespace Util {

    template<class D, class T>
    void parallel(const D& data, T& t)
    {
        std::list<std::future<void>> li;
        for (auto& x : data) {
            li.emplace_back(std::async(std::launch::async, [&x, &t](){
                t(x);
            }));
        }
        for (auto& x : li) {
            x.wait();
        }
    }

    template<class D, class T>
    void pipeline(const D& data, T& t, int thr_count = -1)
    {
        std::list<std::future<void>> li;
        std::mutex mutex;
        typedef std::unique_lock<std::mutex> Lock;

        for (auto& x : data) {
            li.emplace_back(std::async(std::launch::async, [x, &t, &mutex](){
                typename T::TmpResult tmp;
                t.work(x, tmp);
                Lock lk(mutex); // FIXME: implement correct pipeline
                t.join(x, tmp);
            }));
        }
        for (auto& x : li) {
            x.wait();
        }
    }

} // namespace Util

#endif /* _TBB_TRAMPOLINE_H__ */

