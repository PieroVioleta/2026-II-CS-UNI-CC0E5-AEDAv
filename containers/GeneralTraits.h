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
