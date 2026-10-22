#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__
#include "LinkedList.h"

template <typename T>
class DoubleLinkedListNode : public LinkedListNode<T> {
    using Node    = DoubleLinkedListNode<T>;
    using NodePtr = Node *;
    DoubleLinkedListNode *m_pPrev = nullptr; // puntero al nodo anterior
public:
    DoubleLinkedListNode() : LinkedListNode<T>(T{}, Ref{}, nullptr), m_pPrev(nullptr) {}
    DoubleLinkedListNode(const T& value, Ref ref, LinkedListNode<T> *pNext, DoubleLinkedListNode *pPrev = nullptr)
        : LinkedListNode<T>(value, ref, pNext), m_pPrev(pPrev) {}
    DoubleLinkedListNode *GetPrev() const { return m_pPrev; }
    void SetPrev(DoubleLinkedListNode *pPrev) { m_pPrev = pPrev; }
};

template <typename T>
using DoubleLinkedListForwardIterator = LinkedListForwardIterator<T>;

template <typename T>
class DoubleLinkedListBackwardIterator : public GeneralIterator<DoubleLinkedListBackwardIterator<T>, DoubleLinkedListNode<T>> {
    using MySelf = DoubleLinkedListBackwardIterator<T>;
    using Parent = GeneralIterator<MySelf, DoubleLinkedListNode<T>>;
public:
    using Parent::Parent;
    DoubleLinkedListBackwardIterator& operator++() {
        Parent::m_ptr = Parent::m_ptr->GetPrev();
        return *this;
    }
};

template <typename T>
struct DoubleLinkedListTraits : public LinkedListAscTraits<T> {
    using Node              = DoubleLinkedListNode<T>;
    using ForwardIterator   = DoubleLinkedListForwardIterator<T>;
    using BackwardIterator  = DoubleLinkedListBackwardIterator<T>;
};

template <typename Traits>
class DoubleLinkedList : public LinkedList<Traits> {
    using Base = LinkedList<Traits>;
public:
    using value_type = typename Base::value_type;
    using NodePtr = typename Base::NodePtr;
    using BackwardIterator = typename Traits::BackwardIterator;

    void updateLinks() override {
        if (!this->m_pRoot)
            return;
        this->m_pTail->m_pNext = this->m_circular ? this->m_pRoot : nullptr;
        NodePtr curr = this->m_pRoot;
        NodePtr prev = this->m_circular ? this->m_pTail : nullptr;
        do {
            curr->SetPrev(prev);
            prev = curr;
            curr = static_cast<NodePtr>(curr->m_pNext);
        } while (curr && curr != this->m_pRoot);
    }

    BackwardIterator rbegin() { return BackwardIterator(this->m_pTail); }
    BackwardIterator rend()   { return BackwardIterator(nullptr); }
};

#endif // __DOUBLELINKEDLIST_H__
