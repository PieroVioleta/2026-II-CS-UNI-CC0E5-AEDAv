#ifndef __GENERAL_TRAITS_H__
#define __GENERAL_TRAITS_H__
#include <functional> // std::less, std::greater
#include "GeneralIterator.h"
#include "GeneralNode.h"

template <typename T, typename _Compare>
struct DefaultTraits {
    using value_type = T;
    using Compare    = _Compare;
};

template <typename T, typename _Compare = std::less<T>>
struct AscendingTraits : public DefaultTraits<T, _Compare> {};

template <typename T, typename _Compare = std::greater<T>>
struct DescendingTraits : public DefaultTraits<T, _Compare> {};

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
struct VectorAscTraits : public AscendingTraits<T> {
    using Node              = GeneralNode<T>;
    using ForwardIterator   = VectorForwardIterator<Node>;  // itera sobre Node, no sobre T
    using BackwardIterator  = VectorBackwardIterator<Node>; // itera sobre Node, no sobre T
};

template <typename T>
struct VectorDescTraits : public DescendingTraits<T> {
    using Node              = GeneralNode<T>;
    using ForwardIterator   = VectorForwardIterator<Node>;
    using BackwardIterator  = VectorBackwardIterator<Node>;
};

template <typename T>
struct MinHeapTraits : public VectorAscTraits<T> {};

template <typename T>
struct MaxHeapTraits : public VectorDescTraits<T> {};

template <typename T>
class BinaryTreeNode;

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

template <typename T>
struct BinaryTreeTraits : public AscendingTraits<T> {
    using Node              = BinaryTreeNode<T>;
    using ForwardIterator   = BinaryTreeForwardIterator<T>;
    using BackwardIterator  = BinaryTreeBackwardIterator<T>;
};

template <typename T>
struct BinaryTreeDescTraits : public DescendingTraits<T> {
    using Node              = BinaryTreeNode<T>;
    using ForwardIterator   = BinaryTreeForwardIterator<T>;
    using BackwardIterator  = BinaryTreeBackwardIterator<T>;
};

#endif // __GENERAL_TRAITS_H__
