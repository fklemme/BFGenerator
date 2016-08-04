/* SCOPE_EXIT (template from Andrei Alexandrescu)
 * See talk: https://www.youtube.com/watch?v=WjTrfoiB0MQ
 */

#pragma once

#include <utility>

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)
#endif

namespace detail {
    template <typename function>
    class scope_guard {
        function m_f;
    public:
        scope_guard(function &&f) : m_f(std::forward<function>(f)) {}
        ~scope_guard() {m_f();}
    };

    enum class scope_guard_helper {};

    template <typename function>
    scope_guard<function> operator+(scope_guard_helper, function &&f) {
        return scope_guard<function>(std::forward<function>(f));
    }
}

#define SCOPE_EXIT \
    auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE) \
        = ::detail::scope_guard_helper() + [&]()
