#ifndef __GENERAL_CONTAINER_H__
#define __GENERAL_CONTAINER_H__
#include <iostream>
#include <type_traits> // is_void_v, invoke_result_t
#include "../foreach.h"
using namespace std;

template <typename Derived, template <typename> class... Mixins>
class Multi : public Mixins<Derived>... {
public:
    Multi() : Mixins<Derived>()... {} // Added by Er
    using Mixins<Derived>::Mixins...;
};

template <typename Derived>
class Printable {
public:
    friend ostream &operator<<(ostream &os, const Derived &c) {
        os << "[";
        bool first = true;
        for (auto it = c.begin(); it != c.end(); ++it) {
            if (!first)
                os << ",";
            os << *it;
            first = false;
        }
        return os << "]";
    }
};

template <typename Derived>
class Readable {
public:
    friend istream &operator>>(istream &is, Derived &c) {
        c.clear();
        char sep = 0;
        if (!(is >> sep) || sep != '[')
            return is;
        while (is >> sep && sep != ']') {
            if (sep != '(')
                return is;
            is.unget();
            typename Derived::Node node;
            if (!(is >> node))
                return is;
            c.push_back(node.getValue(), node.getRef());
            if (!(is >> sep) || sep != ',')
                return is;
        }
        return is;
    }
};

template <typename Derived>
class Applicable {
public:
    template <typename Func, typename... Args>
    void ApplyFunction(Func func, Args... args) {
        call(func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) FirstThat(Func func, Args... args) {
        return call(func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) call(Func func, Args&&... args) {
        lock_guard<mutex> lock(static_cast<Derived *>(this)->getMutex());
        if constexpr (is_void_v<invoke_result_t<Func, typename Derived::Node &, Args...>>)
            ::call(static_cast<Derived *>(this)->begin(), static_cast<Derived *>(this)->end(), std::forward<Func>(func), std::forward<Args>(args)...);
        else // return type is not void:
            return ::call(static_cast<Derived *>(this)->begin(), static_cast<Derived *>(this)->end(), std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) rcall(Func func, Args&&... args)
    requires requires { static_cast<Derived *>(this)->rbegin(); } {
        lock_guard<mutex> lock(static_cast<Derived *>(this)->getMutex());
        if constexpr (is_void_v<invoke_result_t<Func, typename Derived::Node &, Args...>>)
            ::call(static_cast<Derived *>(this)->rbegin(), static_cast<Derived *>(this)->rend(), std::forward<Func>(func), std::forward<Args>(args)...);
        else // return type is not void:
            return ::call(static_cast<Derived *>(this)->rbegin(), static_cast<Derived *>(this)->rend(), std::forward<Func>(func), std::forward<Args>(args)...);
    }
};

template <typename Container, typename Func, typename... Args>
decltype(auto) rcall(Container &c, Func func, Args&&... args)
requires requires { c.rbegin(); } {
    lock_guard<mutex> lock(c.getMutex());
    if constexpr (is_void_v<invoke_result_t<Func, typename Container::Node &, Args...>>)
        ::call(c.rbegin(), c.rend(), std::forward<Func>(func), std::forward<Args>(args)...);
    else
        return ::call(c.rbegin(), c.rend(), std::forward<Func>(func), std::forward<Args>(args)...);
}

#endif // __GENERAL_CONTAINER_H__
