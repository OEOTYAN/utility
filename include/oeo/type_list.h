#pragma once

#include "utility.h"

namespace oeo {

template <class... Ts>
class type_list {
public:
    template <class T>
    static constexpr bool contains = (std::is_same_v<T, Ts> || ...);

    template <template <class> class T>
    static constexpr bool all = (T<Ts>::value && ...);

    template <template <class> class T>
    static constexpr bool any = (T<Ts>::value || ...);

    static constexpr size_t size = sizeof...(Ts);

    template <template <class> class W>
    using wrap = type_list<W<Ts>...>;

    template <template <class> class M>
    using map = type_list<typename M<Ts>::type...>;

    template <class T>
    using push_back = type_list<Ts..., T>;

    template <class T>
    using push_front = type_list<T, Ts...>;

    template <template <class...> class U>
    using to = U<Ts...>;

    template <template <class...> class U, class... Us>
    using apply_type = U<Us..., Ts...>;

    template <class Fn>
    static void constexpr for_each(Fn&& fn) {
        unroll_type<Ts...>(std::forward<Fn>(fn));
    }
};

} // namespace oeo
