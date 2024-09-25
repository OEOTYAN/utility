#pragma once

#include <atomic>
#include <functional>

namespace oeo {

template <class Fn = std::function<void()>>
class cancellable_callback {
    Fn               callback;
    std::atomic_bool available{true};

public:
    cancellable_callback(Fn&& callback) : callback(std::move(callback)) {}

    bool cancel() { return available.exchange(false); }

    bool try_call() {
        if (available.exchange(false)) {
            callback();
            return true;
        }
        return false;
    }
    template <class F>
    bool move_to(F&& f) {
        if (available.exchange(false)) {
            return f(std::move(callback));
        }
        return false;
    }
};

} // namespace oeo
