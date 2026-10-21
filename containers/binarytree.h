#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__
#include <mutex>
#include <utility> // std::exchange
#include "GeneralTraits.h"
#include "GeneralNode.h"
#include "GeneralIterator.h"
#include "GeneralContainer.h"

template <typename T>
class BinaryTreeNode : public GeneralNode<T> {
public:
    using value_type        = T;
    using MySelf            = BinaryTreeNode<T>;
    using Node              = MySelf;
    using NodePtr           = Node*;
    using Parent            = GeneralNode<T>;
    using Parent::Parent; // Inherit constructor

    NodePtr m_pParent = nullptr;
    NodePtr m_pChild[2] = {nullptr, nullptr}; // left and right children

};

template <typename Traits>
class BinaryTree : public Multi<BinaryTree<Traits>, Printable, Readable, Applicable> {
public:
    using value_type        = typename Traits::value_type;
    using Node              = typename Traits::Node;
    using NodePtr           = Node *;
    using ForwardIterator   = typename Traits::ForwardIterator;
    using BackwardIterator  = typename Traits::BackwardIterator;
    using Compare           = typename Traits::Compare;

private:
    NodePtr m_root = nullptr; // root of the binary tree
    mutable std::mutex m_mutex; // mutex para sincronización
    Compare m_comp; // comparador para ordenar los nodos del arbol

    void internal_insert(NodePtr& node, const value_type& value, Ref ref, NodePtr parent = nullptr) {
        if (!node) {
            node = new Node(value, ref);
            node->m_pParent = parent;
            return;
        }
        auto branch = m_comp(node->getValue(), value);
        internal_insert(node->m_pChild[branch], value, ref, node); // Tail recursion to insert in the correct branch
    }
    void internal_clear(NodePtr& node) {
        if (!node)
            return;
        internal_clear(node->m_pChild[0]);
        internal_clear(node->m_pChild[1]);
        delete node;
        node = nullptr;
    }
    void internal_clone(NodePtr& node, NodePtr other, NodePtr parent) {
        if (!other)
            return;
        node = new Node(other->getValue(), other->getRef());
        node->m_pParent = parent;
        internal_clone(node->m_pChild[0], other->m_pChild[0], node);
        internal_clone(node->m_pChild[1], other->m_pChild[1], node);
    }
public:
    BinaryTree() {}
    BinaryTree(const BinaryTree& another){ *this = another; } // copia profunda del arbol
    BinaryTree(BinaryTree&& another) : m_root(std::exchange(another.m_root, nullptr)) {}
    BinaryTree& operator=(const BinaryTree& another) {
        if (this != &another) {
            clear();
            lock_guard<mutex> lock(another.m_mutex);
            internal_clone(m_root, another.m_root, nullptr);
        }
        return *this;
    }
    BinaryTree& operator=(BinaryTree&& another) {
        if (this != &another) {
            clear();
            m_root = std::exchange(another.m_root, nullptr);
        }
        return *this;
    }
    ~BinaryTree(){ clear(); }

    void clear() {
        scoped_lock lock(m_mutex);
        internal_clear(m_root);
    }

    void insert(const value_type& value, Ref ref) {
        scoped_lock lock(m_mutex);
        internal_insert(m_root, value, ref);
    }
    void push_back(const value_type& value, Ref ref) { insert(value, ref); }

    bool empty() const { return m_root == nullptr; }

    ForwardIterator begin() const {
        NodePtr p = m_root;
        if (p)
            while (p->m_pChild[0])
                p = p->m_pChild[0];
        return ForwardIterator(p);
    }
    ForwardIterator end() const { return ForwardIterator(nullptr); }

    BackwardIterator rbegin() const {
        NodePtr p = m_root;
        if (p)
            while (p->m_pChild[1])
                p = p->m_pChild[1];
        return BackwardIterator(p);
    }
    BackwardIterator rend() const { return BackwardIterator(nullptr); }

    mutex &getMutex() { return m_mutex; }
};
#endif // __BINARYTREE_H__
