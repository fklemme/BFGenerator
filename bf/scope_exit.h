#pragma once

#include <utility>

// ----- Scope_Exit (template from Andrei Alexandrescu) ------------------------
// https://www.youtube.com/watch?v=WjTrfoiB0MQ

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)
#endif

namespace detail {
    enum class scope_guard_helper {};

    template <typename fun>
    class scope_guard {
        fun m_fn;
    public:
        scope_guard(fun &&fn) : m_fn(std::forward<fun>(fn)) {}
        ~scope_guard() {m_fn();}
    };

    template <typename fun>
    scope_guard<fun> operator+(scope_guard_helper, fun &&fn) {
        return scope_guard<fun>(std::forward<fun>(fn));
    }
}

#define SCOPE_EXIT \
    auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE) \
        = ::detail::scope_guard_helper() + [&]()
