#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__
#include "AVLTree.h"

template <typename K, typename M>
struct Par {
    using first_type        = K;
    using second_type       = M;
    K first;
    M second;
    friend std::ostream& operator<<(std::ostream& os, const Par& p) {
        os << p.first << "," << p.second;
        return os;
    }
};

template <typename P>
struct ParLess {
    bool operator()(const P& a, const P& b) const { return a.first < b.first; }
};

template <typename P>
struct ParGreater {
    bool operator()(const P& a, const P& b) const { return a.first > b.first; }
};

template <typename K, typename M = Ref>
struct HashTraits {
    using Node              = AVLNode<Par<K, M>>;
    using ForwardIterator   = BinaryTreeIterator<Node, InOrder, Asc>;
    using BackwardIterator  = BinaryTreeIterator<Node, InOrder, Desc>;
};

template <typename K, typename M = Ref>
struct HashAscTraits : public HashTraits<K, M> {
    using value_type        = Par<K, M>;
    using Compare           = ParLess<Par<K, M>>;
};

template <typename K, typename M = Ref>
struct HashDescTraits : public HashTraits<K, M> {
    using value_type        = Par<K, M>;
    using Compare           = ParGreater<Par<K, M>>;
};

template <typename Traits>
class HashTable : public AVLTree<Traits> {
public:
    using Parent = AVLTree<Traits>;
    using typename Parent::value_type;
    using typename Parent::Node;
    using typename Parent::NodePtr;
    using typename Parent::Compare;
    using Key    = typename value_type::first_type;
    using Mapped = typename value_type::second_type;

    Mapped& operator[](const Key& key) {
        std::lock_guard lock(this->m_mutex);
        NodePtr n = find_key(this->m_root, key);
        if (!n)
            n = this->insert_unlocked(value_type{key, Mapped{}}, Ref{});
        return n->value().second;
    }

    friend std::ostream& operator<<(std::ostream& os, const HashTable& h) {
        for (auto it = h.begin(); it != h.end(); ++it)
            os << (*it).getValue().first << " " << (*it).getValue().second << " ";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, HashTable& h) {
        Key k{};
        Mapped v{};
        std::lock_guard lock(h.m_mutex);
        while (is >> k >> v)
            h.insert_unlocked(value_type{k, v}, Ref{});
        return is;
    }

    void PrintNode(std::ostream& os, NodePtr node, Color color) const override {
        std::ostringstream label;
        label << "(" << node->getValue().first << "," << node->getValue().second
              << ") h=" << node->m_height;
        PrintColored(os, label.str(), color, false);
    }

private:
    NodePtr find_key(NodePtr node, const Key& key) const {
        value_type probe{key, Mapped{}};
        while (node) {
            if (Compare()(probe, node->getValue()))
                node = node->m_pChild[0];
            else if (Compare()(node->getValue(), probe))
                node = node->m_pChild[1];
            else
                break;
        }
        return node;
    }
};

#endif // __HASHTABLE_H__
