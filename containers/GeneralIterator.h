#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__

#include <iostream>
#include <stdexcept>
#include <algorithm> // para std::swap

template <typename Iterator>
class GeneralIterator {
public:
    using value_type        = typename Iterator::value_type;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type *;
    using reference         = value_type&;
protected:
    pointer m_ptr;
public:
    GeneralIterator(pointer ptr) : m_ptr(ptr) {}
    // reference   operator*()   const { return *m_ptr; }
    value_type& operator*() const { return *m_ptr; }
    // value_type* operator->()      { return m_ptr; }
    // friend bool operator== (const GeneralIterator& a, const GeneralIterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };
};

#endif // __GENERAL_ITERATOR_H__