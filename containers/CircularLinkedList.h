#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__
#include "LinkedList.h"

template <typename T>
class CircularForwardIterator : public GeneralIterator<CircularForwardIterator<T>, LinkedListNode<T>> {
    using NodePtr = LinkedListNode<T> *;
    NodePtr m_start;
public:
    using value_type        = LinkedListNode<T>;
    using MySelf            = CircularForwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, value_type>;

    CircularForwardIterator(NodePtr ptr) : Parent(ptr), m_start(ptr) {}

    CircularForwardIterator& operator++() {
        Parent::m_ptr = Parent::m_ptr->m_pNext;
        if (Parent::m_ptr == m_start)
            Parent::m_ptr = nullptr;
        return *this;
    }
};

template <typename T>
struct CircularTraits : public LinkedListAscTraits<T> {
    using ForwardIterator   = CircularForwardIterator<T>;
};

template <typename T>
class CircularLinkedList : public LinkedList<CircularTraits<T>> {
public:
    CircularLinkedList() { this->m_circular = true; }
};

#endif // __CIRCULARLINKEDLIST_H__
