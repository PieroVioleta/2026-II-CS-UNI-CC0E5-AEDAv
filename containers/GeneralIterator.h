#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__

#include <iostream>
#include <stdexcept>
#include <algorithm> // para std::swap

// CRTP
template <typename Derived, typename T>
class GeneralIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = value_type *;
    using reference         = value_type&;
protected:
    pointer m_ptr;
public:
    GeneralIterator(pointer ptr) : m_ptr(ptr) {}
    value_type& operator*() const { return *m_ptr; }
    // value_type* operator->()      { return m_ptr; }
    friend bool operator== (const Derived& a, const Derived& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const Derived& a, const Derived& b) { return a.m_ptr != b.m_ptr; };
};

template <typename T>
class BinaryTreeNode;

template <typename T>
class VectorForwardIterator : public GeneralIterator<VectorForwardIterator<T>, T> {
public:
    using value_type        = T;
    using MySelf            = VectorForwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, T>;
    using Parent::Parent; // Inherit constructor

    // Prefix increment
    VectorForwardIterator& operator++() { ++Parent::m_ptr; return *this; }
};

template <typename T>
class VectorBackwardIterator : public GeneralIterator<VectorBackwardIterator<T>, T> {
public:
    using value_type        = T;
    using MySelf            = VectorBackwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, T>;
    using Parent::Parent; // Inherit constructor

    // Prefix increment
    VectorBackwardIterator& operator++() { --Parent::m_ptr; return *this; }
};

template <typename T>
class BinaryTreeForwardIterator : public GeneralIterator<BinaryTreeForwardIterator<T>, BinaryTreeNode<T>> {
    using NodePtr = BinaryTreeNode<T> *;
public:
    using MySelf            = BinaryTreeForwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, BinaryTreeNode<T>>;
    using Parent::Parent;
    BinaryTreeForwardIterator& operator++() {
        NodePtr p = Parent::m_ptr;
        if (p->m_pChild[1]) {
            p = p->m_pChild[1];
            while (p->m_pChild[0])
                p = p->m_pChild[0];
        } else {
            NodePtr parent = p->m_pParent;
            while (parent && p == parent->m_pChild[1]) {
                p = parent;
                parent = parent->m_pParent;
            }
            p = parent;
        }
        Parent::m_ptr = p;
        return *this;
    }
};

template <typename T>
class BinaryTreeBackwardIterator : public GeneralIterator<BinaryTreeBackwardIterator<T>, BinaryTreeNode<T>> {
    using NodePtr = BinaryTreeNode<T> *;
public:
    using MySelf            = BinaryTreeBackwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, BinaryTreeNode<T>>;
    using Parent::Parent;
    BinaryTreeBackwardIterator& operator++() {
        NodePtr p = Parent::m_ptr;
        if (p->m_pChild[0]) {
            p = p->m_pChild[0];
            while (p->m_pChild[1])
                p = p->m_pChild[1];
        } else {
            NodePtr parent = p->m_pParent;
            while (parent && p == parent->m_pChild[0]) {
                p = parent;
                parent = parent->m_pParent;
            }
            p = parent;
        }
        Parent::m_ptr = p;
        return *this;
    }
};

#endif // __GENERAL_ITERATOR_H__