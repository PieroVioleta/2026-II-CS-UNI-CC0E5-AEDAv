#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <mutex>
#include "GeneralNode.h"
#include "GeneralIterator.h"
#include "../foreach.h"

template <typename T>
class LinkedListNode : public GeneralNode<T> {
    using Node    = LinkedListNode<T>;
    using NodePtr = Node *;
public:
    Node *m_pNext = nullptr; // puntero al siguiente nodo
    LinkedListNode(const T& value, Ref ref, Node *pNext) : GeneralNode<T>(value, ref), m_pNext(pNext){}
};

template <typename T>
class LinkedListForwardIterator : public GeneralIterator<LinkedListForwardIterator<T>, LinkedListNode<T>> {
public:
    using value_type        = LinkedListNode<T>;
    using MySelf            = LinkedListForwardIterator<T>;
    using Parent            = GeneralIterator<MySelf, value_type>;
    using Parent::Parent; // Inherit constructor
    LinkedListForwardIterator& operator++() { Parent::m_ptr = Parent::m_ptr->m_pNext; return *this; }
};

template <typename T>
struct LinkedListAscTraits {
    using value_type        = T;
    using Node              = LinkedListNode<T>;
    using ForwardIterator   = LinkedListForwardIterator<T>;  // itera sobre Node, no sobre T
};

template <typename Traits>
class LinkedList {
public:
    using value_type        = typename Traits::value_type;
    using Node              = typename Traits::Node;
    using NodePtr           = Node *;
    using ForwardIterator   = typename Traits::ForwardIterator;
private:
    NodePtr m_pRoot = nullptr; // puntero al primer nodo de la lista enlazada
    NodePtr m_pTail = nullptr; // puntero al último nodo de la lista enlazada
    std::mutex m_mutex;             // mutex para sincronización

    NodePtr GetRoot() const { return m_pRoot; }
public:
    LinkedList() {}
    LinkedList(const LinkedList&)            = delete; // no se permite copia
    LinkedList& operator=(const LinkedList&) = delete; // no se permite asignacion

    void clear();
    virtual ~LinkedList();

    void push_back(const value_type& value, Ref ref) {
        lock_guard<mutex> lock(m_mutex);
        NodePtr pNew = new Node(value, ref, nullptr);
        if (m_pTail == nullptr)
            m_pRoot = pNew;
        else
            m_pTail->m_pNext = pNew;
        m_pTail = pNew;
    }

private:
    void internalInsert(const value_type& value, Ref ref, NodePtr&rParent);
public:
    void insert(const value_type& value, Ref ref){
        lock_guard<mutex> lock(m_mutex);
        internalInsert(value, ref, m_pRoot);
    }
    
    // TODO: persistencia: write() y read() para LinkedList
    std::ostream &write(std::ostream &os) { return os << *this; }
    std::istream &read(std::istream &is)  { return is >> *this; }
    friend std::ostream &operator <<(std::ostream &os, const LinkedList<Traits> &list) {
        NodePtr current = list.m_pRoot;
        os << "[";
        while (current != nullptr) {
            os << *current;
            current = current->m_pNext;
            if (current != nullptr) os << ",";
        }
        return os << "]";
    }
    // TODO: implementar
    friend std::istream &operator >>(std::istream &is, LinkedList<Traits> &list) {
        return is;
    }
    // Iterators
    ForwardIterator begin() { return ForwardIterator(m_pRoot); }
    ForwardIterator end()   { return ForwardIterator(nullptr); }

    // TODO: implementar ApplyFunction(), FirstThat(), call, rcall para LinkedList
    // Chequear que hago para evitar codigo repetido
};


template <typename Traits>
void LinkedList<Traits>::clear() {
    lock_guard<mutex> lock(m_mutex);
    while (m_pRoot != nullptr) {
        NodePtr pNext = m_pRoot->m_pNext;
        delete m_pRoot;
        m_pRoot = pNext;
    }
    m_pTail = nullptr;
}

template <typename Traits>
LinkedList<Traits>::~LinkedList() {
    clear();
}

// TODO: explicar recursividad de cola de llamadas en insert() y internalInsert()
template <typename Traits>
void LinkedList<Traits>::internalInsert(const value_type& value, Ref ref, NodePtr &rParent){
    if( rParent == nullptr || value < rParent->getValue() ) {
        rParent = new Node(value, ref, rParent);
        return;
    } 
    internalInsert(value, ref, rParent->m_pNext);
}
#endif // __LINKEDLIST_H__
