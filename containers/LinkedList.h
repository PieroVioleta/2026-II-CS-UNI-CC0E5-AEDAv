#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <mutex>
#include "GeneralTraits.h"
#include "GeneralNode.h"
#include "GeneralIterator.h"
#include "GeneralContainer.h"
#include "../foreach.h"

template <typename T>
class LinkedListNode : public GeneralNode<T> {
    using Node = LinkedListNode<T>;
    using NodePtr = Node*;
public:
    Node* m_pNext = nullptr; // puntero al siguiente nodo, npara acceso directo necesita ser publico
    LinkedListNode() : GeneralNode<T>(T{}, Ref{}), m_pNext(nullptr) {}
    // añadir friend class añade otro typename al template
    LinkedListNode(const T& value, Ref ref, Node* pNext) : GeneralNode<T>(value, ref), m_pNext(pNext) {}
};

template <typename T>
class LinkedListForwardIterator : public GeneralIterator<LinkedListForwardIterator<T>, LinkedListNode<T>> {
public:
    using Node = LinkedListNode<T>;
    using MySelf = LinkedListForwardIterator<T>;
    using Parent = GeneralIterator<MySelf, Node>;
    using Parent::Parent; // Inherit constructor
    LinkedListForwardIterator& operator++() { Parent::m_ptr = Parent::m_ptr->m_pNext; return *this; }
};

template <typename T>
struct LinkedListAscTraits : public AscendingTraits<T> {
    using Node              = LinkedListNode<T>;
    using ForwardIterator   = LinkedListForwardIterator<T>;  // itera sobre Node, no sobre T
};

template <typename T>
struct LinkedListDescTraits : public DescendingTraits<T> {
    using Node              = LinkedListNode<T>;
    using ForwardIterator   = LinkedListForwardIterator<T>;  // itera sobre Node, no sobre T
};
template <typename Traits>
class LinkedList : public Multi<LinkedList<Traits>, Printable, Readable, Applicable> {
public:
    using value_type        = typename Traits::value_type;
    using Node              = typename Traits::Node;
    using NodePtr           = Node *;
    using ForwardIterator   = typename Traits::ForwardIterator;
    using Compare           = typename Traits::Compare;
    using Delim             = typename Node::Delim;
    NodePtr GetRoot() const { return m_pRoot; }
protected:
    NodePtr m_pRoot = nullptr; // puntero al primer nodo de la lista enlazada
    NodePtr m_pTail = nullptr; // puntero al último nodo de la lista enlazada
    
    Compare m_comp; // comparador para ordenar los nodos de la lista
    mutable std::mutex m_mutex; // mutex para sincronización
    bool m_circular = false; // activa el cierre de anillo tras cada mutacion

    void openRing() { if (m_pTail) m_pTail->m_pNext = nullptr; }
    void closeRing() { if (m_pTail) m_pTail->m_pNext = m_pRoot; }

    virtual void updateLinks() {
        if (m_circular && m_pTail)
            m_pTail->m_pNext = m_pRoot;
    }

    template <typename NodePtrT>
    void internalInsert(const value_type& value, Ref ref, NodePtrT& rParent) {
        if (rParent == nullptr || value < rParent->getValue()) {
            NodePtr newNode = new Node(value, ref, rParent);
            rParent = newNode;
            if (newNode->m_pNext == nullptr)
                m_pTail = newNode;
            return;
        }
        internalInsert(value, ref, rParent->m_pNext);
    }

public:
    LinkedList() {}
    LinkedList(const LinkedList& another){ *this = another; } // copia profunda de la lista enlazada
    LinkedList& operator=(const LinkedList& another) {
        clear();
        if (!another.m_pRoot)
            return *this;
        std::lock_guard<std::mutex> lock(another.m_mutex);

        m_pRoot = new Node(another.GetRoot()->getValue(), another.GetRoot()->getRef(), nullptr);

        NodePtr next = another.m_pRoot->m_pNext;
        NodePtr curr = m_pRoot;

        while (next) {
            curr->m_pNext = new Node(next->getValue(), next->getRef(), nullptr);
            curr = curr->m_pNext;
            next = next->m_pNext;
        }
        m_pTail = curr;
        updateLinks();
        return *this;
    }
    LinkedList(initializer_list<pair<value_type, Ref>> values) {
        for (const auto &v : values)
            push_back(v.first, v.second);
    }

    virtual void clear() {
        lock_guard<mutex> lock(m_mutex);
        if (m_pTail != nullptr)
            m_pTail->m_pNext = nullptr;
        NodePtr current = m_pRoot;
        while (current != nullptr) {
            NodePtr pNext = static_cast<NodePtr>(current->m_pNext); // todos los nodos son del tipo de Traits
            delete current;
            current = pNext;
        }
        m_pRoot = nullptr;
        m_pTail = nullptr;
    }
    virtual ~LinkedList(){ clear(); };

    virtual void push_back(const value_type& value, Ref ref) {
        scoped_lock lock(m_mutex);
        NodePtr new_node = new Node(value, ref, nullptr);
        if (!m_pRoot) {
            m_pRoot = new_node;
            m_pTail = new_node;
        } else {
            m_pTail->m_pNext = new_node;
            m_pTail = new_node;
        }
        updateLinks();
    }

    bool empty() const { return m_pRoot == nullptr; }

    void insert(const value_type& value, Ref ref) {
        scoped_lock lock(m_mutex);
        openRing();
        internalInsert(value, ref, m_pRoot);
        updateLinks();
    }

    std::ostream& write(std::ostream& os) { return os << *this; }
    std::istream& read(std::istream& is) { return is >> *this; }

    ForwardIterator begin() const { return ForwardIterator(m_pRoot); }
    ForwardIterator end() const { return ForwardIterator(nullptr); }

    mutex &getMutex() { return m_mutex; }
};
#endif // __LINKEDLIST_H__
