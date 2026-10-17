#ifndef __GENERAL_STREAMABLE_H__
#define __GENERAL_STREAMABLE_H__
#include <iostream>
using namespace std;

template <typename Derived, template <typename> class... Mixins>
class Multi : public Mixins<Derived>... {
public:
    Multi() : Mixins<Derived>()... {}
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

#endif // __GENERAL_STREAMABLE_H__
