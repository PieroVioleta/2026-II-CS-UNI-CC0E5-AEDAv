#ifndef __MERKLETREE_H__
#define __MERKLETREE_H__
#include "binarytree.h"
#include "vector.h"
#include <iomanip>
#include <string>

// FNV-1a de 64 bits (aritmetica mod 2^64)
inline TXX HashFnv(const std::string& data) {
    TXX h = 0xcbf29ce484222325ULL;
    for (TXX ch : data) {
        h ^= ch;
        h *= 0x100000001b3ULL;
    }
    return h;
}

template <typename T>
class MerkleNode : public BaseBinaryNode<T, MerkleNode<T>> {
public:
    using Parent = BaseBinaryNode<T, MerkleNode<T>>;
    using Parent::Parent;
    bool m_leaf = false;
};

template <typename T>
struct MerkleTraits {
    using Node              = MerkleNode<T>;
    using ForwardIterator   = BinaryTreeIterator<Node, InOrder, Asc>;
    using BackwardIterator  = BinaryTreeIterator<Node, InOrder, Desc>;
};

template <typename T>
struct MerkleAscTraits : public AscendingTraits<T>, public MerkleTraits<T> {};

template <typename Traits>
class MerkleTree : public BaseBinaryTree<Traits> {
public:
    using Parent = BaseBinaryTree<Traits>;
    using typename Parent::value_type;
    using typename Parent::Node;
    using typename Parent::NodePtr;

    // hash de la raiz
    TXX Root() const { return this->m_root ? this->m_root->getValue() : 0; }

    // construye bottom-up: hoja = hash del bloque, interno = hash(hijo0|hijo1)
    void Build(const Vector<VectorAscTraits<std::string>>& blocks) {
        std::lock_guard lock(this->m_mutex);
        this->clear();
        Vector<VectorAscTraits<NodePtr>> level; // punteros del nivel actual
        for (const auto& b : blocks) {
            NodePtr leaf = new Node(HashFnv(b.getValue()), level.size());
            leaf->m_leaf = true;
            level.push_back(leaf, level.size());
        }
        if (level.size() == 0)
            return;
        while (level.size() > 1) {
            if (level.size() % 2) { // impar: duplica la ultima hoja
                NodePtr last = level[level.size()-1].getValue();
                level.push_back(new Node(last->getValue(), last->getRef()), level.size());
            }
            for (size_t i = 0; i < level.size() / 2; ++i) { // reduce en sitio
                NodePtr l = level[2*i].getValue();
                NodePtr r = level[2*i+1].getValue();
                NodePtr p = new Node(
                    HashFnv(std::to_string(l->getValue()) + "|" + std::to_string(r->getValue())), i);
                p->m_pChild[0] = l;
                p->m_pChild[1] = r;
                l->m_parent = p;
                r->m_parent = p;
                level[i] = GeneralNode<NodePtr>(p, i);
            }
            size_t extra = level.size() - level.size() / 2;
            for (size_t i = 0; i < extra; ++i)
                level.pop_back(); // recorta los hash viejos de la cola
        }
        this->m_root = level[0].getValue();
    }

    // label: hash en hex (8 digitos) + indice de hoja
    void PrintNode(std::ostream& os, NodePtr node, Color color) const override {
        std::ostringstream label;
        label << "h:" << std::hex << std::setfill('0') << std::setw(8)
              << (node->getValue() & 0xffffffffLL);
        if (node->m_leaf)
            label << " [" << node->getRef() << "]";
        PrintColored(os, label.str(), color, false);
    }
};

#endif // __MERKLETREE_H__
