#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__
#include "GeneralNode.h"
#include "GeneralIterator.h"
#include "GeneralContainer.h"
#include "mixins.h"
#include "../utilities/printcolored.h"
#include <string>

template <typename T>
class BinaryTreeNode : public GeneralNode<T> {
public:
    using value_type        = T;
    using MySelf            = BinaryTreeNode<T>;
    using Node              = MySelf;
    using NodePtr           = Node*;
    using Parent            = GeneralNode<T>;
    using Parent::Parent; // Inherit constructor

    NodePtr m_pChild[2] = {nullptr, nullptr}; // left and right children
    NodePtr m_parent = nullptr;

};

// Un solo iterador para los 6 recorridos: (inorden, preorden, postorden) x (asc, desc).
// Order selecciona el orden de visita y Dir espeja izquierda/derecha.
enum TraversalOrder : TX { InOrder = 0, PreOrder = 1, PostOrder = 2 };

template <typename T, TX Order = InOrder, TX Dir = 0>
class BinaryTreeIterator : public GeneralIterator<BinaryTreeIterator<T, Order, Dir>, BinaryTreeNode<T>> {
public:
    using Node = BinaryTreeNode<T>;
    using MySelf = BinaryTreeIterator<T, Order, Dir>;
    using Parent = GeneralIterator<MySelf, Node>;
    using NodePtr = Node*;
    using Parent::Parent;
    NodePtr m_parent = nullptr;

    MySelf& operator++();
};

template <typename T, TX Order, TX Dir>
auto BinaryTreeIterator<T, Order, Dir>::operator++() -> MySelf& {
    auto node = Parent::m_ptr;
    if (node == nullptr)
        return *this;
    NodePtr parent = nullptr;
    switch (Order) {
        case InOrder:
            if (node->m_pChild[1-Dir]) {
                node = node->m_pChild[1-Dir];
                while (node->m_pChild[Dir])
                    node = node->m_pChild[Dir];
                Parent::m_ptr = node;
            } else {
                parent = node->m_parent;
                while (parent && node == parent->m_pChild[1-Dir]) {
                    node = parent;
                    parent = parent->m_parent;
                }
                Parent::m_ptr = parent;
            }
            break;
        case PreOrder:
            if (node->m_pChild[Dir])
                Parent::m_ptr = node->m_pChild[Dir];
            else if (node->m_pChild[1-Dir])
                Parent::m_ptr = node->m_pChild[1-Dir];
            else {
                for (parent = node->m_parent; parent; node = parent, parent = parent->m_parent)
                    if (node == parent->m_pChild[Dir] && parent->m_pChild[1-Dir])
                        break;
                Parent::m_ptr = parent ? parent->m_pChild[1-Dir] : nullptr;
            }
            break;
        default:
            parent = node->m_parent;
            if (!parent)
                Parent::m_ptr = nullptr; // la raiz cierra el recorrido
            else if (node == parent->m_pChild[1-Dir] || !parent->m_pChild[1-Dir])
                Parent::m_ptr = parent;
            else {
                node = parent->m_pChild[1-Dir];
                while (node->m_pChild[Dir] || node->m_pChild[1-Dir])
                    node = node->m_pChild[node->m_pChild[Dir] ? Dir : 1-Dir];
                Parent::m_ptr = node;
            }
            break;
    }
    return *this;
}

template <typename T>
using BinaryTreeForwardIterator = BinaryTreeIterator<T, InOrder, 0>;

template <typename T>
using BinaryTreeBackwardIterator = BinaryTreeIterator<T, InOrder, 1>;

template <typename T>
struct BinaryTreeTraits {
    using Node              = BinaryTreeNode<T>;
    using ForwardIterator   = BinaryTreeForwardIterator<T>;
    using BackwardIterator  = BinaryTreeBackwardIterator<T>;
};

template <typename T>
struct BinaryTreeAscTraits : public AscendingTraits<T>, public BinaryTreeTraits<T> {};

template <typename T>
struct BinaryTreeDescTraits : public DescendingTraits<T>, public BinaryTreeTraits<T> {};

template <typename Traits, template <typename> typename... Mixins>
class BaseBinaryTree;

template <typename Traits>
using BinaryTree = BaseBinaryTree<Traits, Callable, Applicable, Searchable, ReverseCallable, Printable, Readable>;

template <typename Traits, template <typename> typename... Mixins>
class BaseBinaryTree : public GeneralContainer<Traits, Mixins...> {
public:
    using MySelf = BaseBinaryTree;
    using Parent = GeneralContainer<Traits, Mixins...>;
    using typename Parent::value_type;
    using typename Parent::Node;
    using typename Parent::NodePtr;
    using typename Parent::ForwardIterator;
    using BackwardIterator = typename Traits::BackwardIterator;
    using Compare = typename Traits::Compare;

protected:
    using Parent::m_mutex;

private:
    NodePtr m_root = nullptr; // root of the binary tree
    void internal_insert(NodePtr& node, const value_type& value, Ref ref, NodePtr parent = nullptr);

    void PrintTree(std::ostream &os, NodePtr node, int depth, const char *connector) const {
        if (!node)
            return;
        static const Color kLevelColors[] = {Color::Red, Color::Green, Color::Yellow, Color::Blue, Color::Magenta, Color::Cyan};
        PrintTree(os, node->m_pChild[1], depth + 1, "┌─ ");
        os << std::string(depth * 4, ' ') << connector;
        PrintColored(os, *node, kLevelColors[depth % 6], false);
        os << "\n";
        PrintTree(os, node->m_pChild[0], depth + 1, "└─ ");
    }

    NodePtr detach_min(NodePtr sub, NodePtr subParent) {
        while (sub->m_pChild[0]) {
            subParent = sub;
            sub = sub->m_pChild[0];
        }
        if (subParent->m_pChild[0] == sub)
            subParent->m_pChild[0] = sub->m_pChild[1];
        else
            subParent->m_pChild[1] = sub->m_pChild[1];
        if (sub->m_pChild[1])
            sub->m_pChild[1]->m_parent = subParent;
        return sub;
    }
    void internal_remove(NodePtr& node, NodePtr parent, const value_type& value, NodePtr* out) {
        if (!node)
            return;
        if (Compare()(value, node->getValue()))
            return internal_remove(node->m_pChild[0], node, value, out);
        if (Compare()(node->getValue(), value))
            return internal_remove(node->m_pChild[1], node, value, out);
        *out = node;
        if (!node->m_pChild[0] || !node->m_pChild[1]) {
            node = node->m_pChild[node->m_pChild[0] ? 0 : 1];
        } else {
            NodePtr succ = detach_min(node->m_pChild[1], node);
            succ->m_pChild[0] = node->m_pChild[0];
            succ->m_pChild[1] = node->m_pChild[1];
            if (succ->m_pChild[0])
                succ->m_pChild[0]->m_parent = succ;
            if (succ->m_pChild[1])
                succ->m_pChild[1]->m_parent = succ;
            succ->m_parent = parent;
            node = succ;
            return;
        }
        if (node)
            node->m_parent = parent;
    }
public:
    BaseBinaryTree() = default;
    BaseBinaryTree(const BaseBinaryTree& another) { *this = another; }
    BaseBinaryTree(BaseBinaryTree&& another) noexcept { *this = std::move(another); }
    virtual ~BaseBinaryTree() { clear(); }

    void insert(const value_type& value, Ref ref) {
        std::lock_guard lock(this->m_mutex);
        internal_insert(m_root, value, ref);
    }
    // Move constructor and move assignment operator
    // Constructor and destructor
    // Persistencia (operator << y operator >> globales para todas las estructuras pero si es para BT con indentacion entonces hacerlo por separado)
    // Concurrencia
    // Iteradores (forward y backward)
    // FirstThat, ForEach, call, rcall por herencia

    void clear() override;

    void push_back(const value_type& value, Ref ref) override { insert(value, ref); }

    Node remove(const value_type& value) {
        std::lock_guard lock(this->m_mutex);
        NodePtr removed = nullptr;
        internal_remove(m_root, nullptr, value, &removed);
        if (!removed)
            return Node();
        Node result(removed->getValue(), removed->getRef());
        delete removed;
        return result;
    }

    void clear_recursive(NodePtr node);

    ForwardIterator begin() const override;
    ForwardIterator end() const override { return ForwardIterator(nullptr); }

    BackwardIterator rbegin() const;
    BackwardIterator rend() const { return BackwardIterator(nullptr); }

    // Primer nodo del recorrido (Order, Dir): la raiz en preorden; en
    // inorden se baja por el hijo Dir; en postorden se prefiere Dir.
    template <TX Order, TX Dir>
    NodePtr first_node(NodePtr node) const {
        if (!node)
            return node;
        if (Order == PreOrder)
            return node;
        if (Order == InOrder) {
            while (node->m_pChild[Dir])
                node = node->m_pChild[Dir];
            return node;
        }
        while (node->m_pChild[Dir] || node->m_pChild[1-Dir])
            node = node->m_pChild[node->m_pChild[Dir] ? Dir : 1-Dir];
        return node;
    }

    // begin/end para cualquiera de los 6 recorridos, p.ej. begin<PreOrder, 1>()
    template <TX Order, TX Dir = 0>
    BinaryTreeIterator<value_type, Order, Dir> begin() const {
        return BinaryTreeIterator<value_type, Order, Dir>(first_node<Order, Dir>(m_root));
    }

    template <TX Order, TX Dir = 0>
    BinaryTreeIterator<value_type, Order, Dir> end() const {
        return BinaryTreeIterator<value_type, Order, Dir>(nullptr);
    }

    NodePtr GetRoot() const { return m_root; }

    virtual void PrintTree(std::ostream &os = std::cout) const {
        PrintTree(os, m_root, 0, "");
    }

    BaseBinaryTree& operator=(const BaseBinaryTree& another);
    BaseBinaryTree& operator=(BaseBinaryTree&& another) noexcept;
};

template <typename Traits, template <typename> typename... Mixins>
void BaseBinaryTree<Traits, Mixins...>::internal_insert(NodePtr& node, const value_type& value, Ref ref, NodePtr parent) {
    if (!node) {
        node = new Node(value, ref);
        node->m_parent = parent;
        return;
    }
    auto branch = Compare()(node->getValue(), value);
    internal_insert(node->m_pChild[branch], value, ref, node); // Tail recursion to insert in the correct branch
}

template <typename Traits, template <typename> typename... Mixins>
void BaseBinaryTree<Traits, Mixins...>::clear() {
    std::lock_guard lock(this->m_mutex);
    clear_recursive(m_root);
    m_root = nullptr;
}

template <typename Traits, template <typename> typename... Mixins>
void BaseBinaryTree<Traits, Mixins...>::clear_recursive(NodePtr node) {
    if (!node) return;
    clear_recursive(node->m_pChild[0]);
    clear_recursive(node->m_pChild[1]);
    delete node;
}

template <typename Traits, template <typename> typename... Mixins>
auto BaseBinaryTree<Traits, Mixins...>::begin() const -> ForwardIterator {
    NodePtr curr = m_root;
    while (curr && curr->m_pChild[0]) curr = curr->m_pChild[0];
    return ForwardIterator(curr);
}

template <typename Traits, template <typename> typename... Mixins>
auto BaseBinaryTree<Traits, Mixins...>::rbegin() const -> BackwardIterator {
    NodePtr curr = m_root;
    while (curr && curr->m_pChild[1]) curr = curr->m_pChild[1];
    return BackwardIterator(curr);
}

template <typename Traits, template <typename> typename... Mixins>
auto BaseBinaryTree<Traits, Mixins...>::operator=(const BaseBinaryTree& another) -> BaseBinaryTree& {
    this->copy_from(another);
    return *this;
}

template <typename Traits, template <typename> typename... Mixins>
auto BaseBinaryTree<Traits, Mixins...>::operator=(BaseBinaryTree&& another) noexcept -> BaseBinaryTree& {
    this->move_from(another, [&]{ m_root = std::exchange(another.m_root, nullptr); });
    return *this;
}

#endif // __BINARYTREE_H__