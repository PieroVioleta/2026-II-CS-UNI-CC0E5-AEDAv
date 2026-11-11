#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__
#include "AVLTree.h"

template <typename T>
struct HashTraits {
    using Node              = AVLNode<T>;
    using ForwardIterator   = BinaryTreeIterator<Node, InOrder, Asc>;
    using BackwardIterator  = BinaryTreeIterator<Node, InOrder, Desc>;
};

template <typename T>
struct HashAscTraits : public AscendingTraits<T>, public HashTraits<T> {};

template <typename T>
struct HashDescTraits : public DescendingTraits<T>, public HashTraits<T> {};

template <typename Traits>
class HashTable : public AVLTree<Traits> {
public:
    using Parent = AVLTree<Traits>;
    using typename Parent::value_type;
    using typename Parent::Node;
    using typename Parent::NodePtr;
    using typename Parent::Compare;

    Ref& operator[](const value_type& key) {
        std::lock_guard lock(this->m_mutex);
        NodePtr n = find_key(this->m_root, key);
        if (!n)
            n = this->insert_unlocked(key, Ref{});
        return n->ref();
    }

    friend std::ostream& operator<<(std::ostream& os, const HashTable& h) {
        for (auto it = h.begin(); it != h.end(); ++it)
            os << *it << ' ';
        return os;
    }

    friend std::istream& operator>>(std::istream& is, HashTable& h) {
        Node n;
        while (is >> n)
            h.insert(n.getValue(), n.getRef());
        return is;
    }

private:
    NodePtr find_key(NodePtr node, const value_type& key) const {
        while (node) {
            if (Compare()(key, node->getValue()))
                node = node->m_pChild[0];
            else if (Compare()(node->getValue(), key))
                node = node->m_pChild[1];
            else
                break;
        }
        return node;
    }
};

#endif // __HASHTABLE_H__
