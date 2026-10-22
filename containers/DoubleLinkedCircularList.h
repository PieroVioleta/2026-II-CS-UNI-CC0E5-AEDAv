#ifndef __DOUBLELINKEDCIRCULARLIST_H__
#define __DOUBLELINKEDCIRCULARLIST_H__
#include "DoubleLinkedList.h"
#include "CircularLinkedList.h"

template <typename T>
using DoubleCircularForwardIterator = CircularForwardIterator<T>;

template <typename T>
class DoubleCircularBackwardIterator : public GeneralIterator<DoubleCircularBackwardIterator<T>, DoubleLinkedListNode<T>> {
    using NodePtr = DoubleLinkedListNode<T> *;
    NodePtr m_start;
public:
    using value_type        = DoubleLinkedListNode<T>;
    using MySelf            = DoubleCircularBackwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, value_type>;

    DoubleCircularBackwardIterator(NodePtr ptr) : Parent(ptr), m_start(ptr) {}

    DoubleCircularBackwardIterator& operator++() {
        Parent::m_ptr = Parent::m_ptr->GetPrev();
        if (Parent::m_ptr == m_start)
            Parent::m_ptr = nullptr;
        return *this;
    }
};

template <typename T>
struct DoubleCircularTraits : public DoubleLinkedListTraits<T> {
    using ForwardIterator   = DoubleCircularForwardIterator<T>;
    using BackwardIterator  = DoubleCircularBackwardIterator<T>;
};

template <typename T>
class DoubleLinkedCircularList : public DoubleLinkedList<DoubleCircularTraits<T>> {
public:
    DoubleLinkedCircularList() { this->m_circular = true; }
};

#endif // __DOUBLELINKEDCIRCULARLIST_H__
