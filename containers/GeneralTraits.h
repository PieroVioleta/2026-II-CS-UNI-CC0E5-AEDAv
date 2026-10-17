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

#endif // __GENERAL_TRAITS_H__
