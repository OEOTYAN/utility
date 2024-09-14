#pragma once

#include <type_traits>
#include <utility>

#ifdef _MSC_VER

#ifndef O_EBO
#define O_EBO __declspec(empty_bases)
#endif

#ifndef O_FORCEINLINE
#define O_FORCEINLINE __forceinline
#endif

#ifndef O_NOINLINE
#define O_NOINLINE __declspec(noinline)
#endif

#ifndef O_RETURN_ADDRESS
#define O_RETURN_ADDRESS _ReturnAddress()
#endif

#ifndef O_CURRENT_LINE
#define O_CURRENT_LINE __builtin_LINE()
#endif

#ifndef O_CURRENT_COLUMN
#define O_CURRENT_COLUMN __builtin_COLUMN()
#endif

#ifndef O_CURRENT_FILE
#define O_CURRENT_FILE __builtin_FILE()
#endif

#ifndef O_CURRENT_FUNCTION
#define O_CURRENT_FUNCTION __builtin_FUNCTION()
#endif

#else

#ifndef O_EBO
#define O_EBO
#endif

#ifndef O_FORCEINLINE
#define O_FORCEINLINE inline __attribute__((always_inline))
#endif

#ifndef O_NOINLINE
#define O_NOINLINE __attribute__((noinline))
#endif

#ifndef O_RETURN_ADDRESS
#define O_RETURN_ADDRESS __builtin_return_address(0)
#endif

#ifndef O_CURRENT_LINE
#define O_CURRENT_LINE __builtin_LINE()
#endif

#ifndef O_CURRENT_COLUMN
#define O_CURRENT_COLUMN 0
#endif

#ifndef O_CURRENT_FILE
#define O_CURRENT_FILE __builtin_FILE()
#endif

#ifndef O_CURRENT_FUNCTION
#define O_CURRENT_FUNCTION __builtin_FUNCTION()
#endif

#endif

#ifdef _MSVC_STL_VERSION

#ifndef O_CONSTEXPR23
#define O_CONSTEXPR23 _CONSTEXPR23
#endif

#ifndef O_UNREACHABLE
#define O_UNREACHABLE _STL_UNREACHABLE
#endif

#else

#ifndef O_CONSTEXPR23
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L
#define O_CONSTEXPR23 constexpr
#else
#define O_CONSTEXPR23 inline
#endif
#endif

#ifndef O_UNREACHABLE
#define O_UNREACHABLE __builtin_unreachable()
#endif

#endif

#ifndef O_MACHINE_PAUSE
#if (defined(_M_IX86) || defined(_M_X64) || defined(__x86_64__)) && !defined(_CHPE_ONLY_) && !defined(_M_ARM64EC)
#define O_MACHINE_PAUSE _mm_pause()
#elif defined(__ARM_ARCH_7A__) || defined(__aarch64__)
#define O_MACHINE_PAUSE __asm__ __volatile__("isb sy" ::: "memory")
#endif
#endif

#ifndef O_UNIQUE_TYPE
#ifdef __INTELLISENSE__
#define O_UNIQUE_TYPE decltype(nullptr)
#else
#define O_UNIQUE_TYPE decltype([] {})
#endif
#endif

namespace oeo {

template <size_t N, int Strategy>
struct visit_strategy;

template <size_t N>
struct visit_strategy<N, -1> {
    template <class T, class... Ts>
    static constexpr std::array<std::decay_t<T>, sizeof...(Ts) + 1> make_visitor_array(T&& t, Ts&&... ts) {
        return {
            {std::forward<T>(t), std::forward<Ts>(ts)...}
        };
    }
    template <class Ret, class Fn, size_t I, class... Args>
    static constexpr Ret invoke_visitor(Fn&& fn, Args&&... args) {
        return std::forward<Fn>(fn).template operator()<I>(std::forward<Args>(args)...);
    }

    template <class Ret, class Fn, class... Args, size_t... Ns>
    static constexpr decltype(auto) make_callers(std::integer_sequence<size_t, Ns...>) {
        return make_visitor_array(&invoke_visitor<Ret, Fn, Ns, Args...>...);
    }
    template <class Ret, class Fn, class... Args>
    static constexpr decltype(auto) callers = make_callers<Ret, Fn, Args...>(std::make_index_sequence<N>());

    template <class Ret, class Fn, class... Args>
    static constexpr Ret invoke(size_t idx, Fn&& fn, Args&&... args) {
        static_assert(N > 256);
        return callers<Ret, Fn, Args...>[idx](std::forward<Fn>(fn), std::forward<Args>(args)...);
    }
};

template <size_t N>
struct visit_strategy<N, 0> {
    template <class Ret, class Fn, class... Args>
    static constexpr Ret invoke(size_t idx, Fn&& fn, Args&&... args) {
        return std::forward<Fn>(fn).template operator()<0>(std::forward<Args>(args)...);
    }
};

#ifndef O_VISIT_CASE
#define O_VISIT_CASE(n)                                                                                                \
    case (n):                                                                                                          \
        if constexpr ((n) < N) {                                                                                       \
            return std::forward<Fn>(fn).template operator()<(n)>(std::forward<Args>(args)...);                         \
        }                                                                                                              \
        O_UNREACHABLE;                                                                                                 \
        [[fallthrough]]

#define O_VISIT_STAMP(stamper, n)                                                                                      \
    static_assert(N > (n) / 4 && N <= (n));                                                                            \
    switch (idx) {                                                                                                     \
        stamper(0, O_VISIT_CASE);                                                                                      \
    default:                                                                                                           \
        O_UNREACHABLE;                                                                                                 \
    }

#define O_STAMP4(n, x)                                                                                                 \
    x(n);                                                                                                              \
    x(n + 1);                                                                                                          \
    x(n + 2);                                                                                                          \
    x(n + 3)
#define O_STAMP16(n, x)                                                                                                \
    O_STAMP4(n, x);                                                                                                    \
    O_STAMP4(n + 4, x);                                                                                                \
    O_STAMP4(n + 8, x);                                                                                                \
    O_STAMP4(n + 12, x)
#define O_STAMP64(n, x)                                                                                                \
    O_STAMP16(n, x);                                                                                                   \
    O_STAMP16(n + 16, x);                                                                                              \
    O_STAMP16(n + 32, x);                                                                                              \
    O_STAMP16(n + 48, x)
#define O_STAMP256(n, x)                                                                                               \
    O_STAMP64(n, x);                                                                                                   \
    O_STAMP64(n + 64, x);                                                                                              \
    O_STAMP64(n + 128, x);                                                                                             \
    O_STAMP64(n + 192, x)

#define O_STAMP(n, x) x(O_STAMP##n, n)

template <size_t N>
struct visit_strategy<N, 1> {
    template <class Ret, class Fn, class... Args>
    static constexpr Ret invoke(size_t idx, Fn&& fn, Args&&... args) {
        O_STAMP(4, O_VISIT_STAMP);
    }
};

template <size_t N>
struct visit_strategy<N, 2> {
    template <class Ret, class Fn, class... Args>
    static constexpr Ret invoke(size_t idx, Fn&& fn, Args&&... args) {
        O_STAMP(16, O_VISIT_STAMP);
    }
};

template <size_t N>
struct visit_strategy<N, 3> {
    template <class Ret, class Fn, class... Args>
    static constexpr Ret invoke(size_t idx, Fn&& fn, Args&&... args) {
        O_STAMP(64, O_VISIT_STAMP);
    }
};

template <size_t N>
struct visit_strategy<N, 4> {
    template <class Ret, class Fn, class... Args>
    static constexpr Ret invoke(size_t idx, Fn&& fn, Args&&... args) {
        O_STAMP(256, O_VISIT_STAMP);
    }
};

#undef O_VISIT_CASE
#undef O_VISIT_STAMP
#undef O_STAMP
#undef O_STAMP256
#undef O_STAMP64
#undef O_STAMP16
#undef O_STAMP4
#endif


template <size_t N>
struct priority_t : priority_t<N - 1> {};

template <>
struct priority_t<0> {};

template <size_t N>
constexpr priority_t<N> priority{};


template <class... Ts>
struct overloaded : Ts... {
    using is_transparent = void;
    using Ts::operator()...;
};
template <class... Ts>
struct opaque_overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts, class Fn>
constexpr void unroll_type(Fn&& fn) {
    [&]<size_t... I>(std::index_sequence<I...>) {
        (void(std::forward<Fn>(fn).template operator()<Ts>(I)), ...);
    }(std::index_sequence_for<Ts...>());
}

template <size_t N, class Fn>
constexpr void unroll(Fn&& fn) {
    [&]<size_t... I>(std::index_sequence<I...>) {
        (void(std::forward<Fn>(fn)(I)), ...);
    }(std::index_sequence_for<Ts...>());
}

template <size_t N, class Fn, class... Args>
constexpr decltype(auto) visit_index(size_t index, Fn&& fn, Args&&... args) {
    constexpr int strategy = N == 1 ? 0 : N <= 4 ? 1 : N <= 16 ? 2 : N <= 64 ? 3 : N <= 256 ? 4 : -1;
    using Impl             = typename visit_strategy<N, strategy>;
    using Ret              = decltype((std::declval<Fn>().template operator()<0>(std::declval<Args>()...)));
    return (Impl::template invoke<Ret>(index, std::forward<Fn>(fn), std::forward<Args>(args)...));
}

template <class Ret, size_t N, class Fn, class... Args>
constexpr Ret visit_index(size_t index, Fn&& fn, Args&&... args) {
    constexpr int strategy = N == 1 ? 0 : N <= 4 ? 1 : N <= 16 ? 2 : N <= 64 ? 3 : N <= 256 ? 4 : -1;
    using Impl             = typename visit_strategy<N, strategy>;
    return Impl::template invoke<Ret>(index, std::forward<Fn>(fn), std::forward<Args>(args)...);
}

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
