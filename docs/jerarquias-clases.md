# Jerarquías de clases del curso

## 1 · Nodos

```mermaid
classDiagram
    class GeneralNode~T~ {
        -T m_value
        -Ref m_ref
        +getValue() T
        +getRef() Ref
        +value() T&
        +operator<<(ostream&, const GeneralNode~T~&)
    }
    class LinkedListNode~T~ {
        +Node* m_pNext
        +LinkedListNode(const T&, Ref, Node*)
    }
    class DoubleLinkedListNode~T~ {
        -DLLNode* m_pPrev
        +GetPrev() DLLNode*
        +SetPrev(DLLNode*)
        +DLLNode(const T&, Ref, LinkedListNode~T~*, DLLNode*)
    }
    class BinaryTreeNode~T~ {
        +Node* m_pParent
        +Node* m_pChild[2]
    }
    GeneralNode~T~ <|-- LinkedListNode~T~
    LinkedListNode~T~ <|-- DoubleLinkedListNode~T~
    GeneralNode~T~ <|-- BinaryTreeNode~T~
```

## 2 · Iteradores (CRTP)

```mermaid
classDiagram
    class GeneralIterator~Derived,T~ {
        #pointer m_ptr
        +GeneralIterator(pointer)
        +operator*() value_type&
        +operator==(const Derived&) bool
        +operator!=(const Derived&) bool
    }
    class VectorForwardIterator~T~ {
        +operator++()
    }
    class VectorBackwardIterator~T~ {
        +operator++()
    }
    class LinkedListForwardIterator~T~ {
        +operator++()
    }
    class CircularForwardIterator~T~ {
        -NodePtr m_start
        +operator++()
    }
    class DoubleLinkedListForwardIterator~T~ {
        +operator++()
    }
    class DoubleLinkedListBackwardIterator~T~ {
        +operator++()
    }
    class DoubleCircularForwardIterator~T~ {
        -NodePtr m_start
        +operator++()
    }
    class DoubleCircularBackwardIterator~T~ {
        -NodePtr m_start
        +operator++()
    }
    class BinaryTreeForwardIterator~T~ {
        +operator++() : sucesor in-order via m_pParent
    }
    class BinaryTreeBackwardIterator~T~ {
        +operator++() : predecesor via m_pParent
    }
    GeneralIterator~Derived,T~ <|-- VectorForwardIterator~T~
    GeneralIterator~Derived,T~ <|-- VectorBackwardIterator~T~
    GeneralIterator~Derived,T~ <|-- LinkedListForwardIterator~T~
    GeneralIterator~Derived,T~ <|-- CircularForwardIterator~T~
    GeneralIterator~Derived,T~ <|-- DoubleLinkedListForwardIterator~T~
    GeneralIterator~Derived,T~ <|-- DoubleLinkedListBackwardIterator~T~
    GeneralIterator~Derived,T~ <|-- DoubleCircularForwardIterator~T~
    GeneralIterator~Derived,T~ <|-- DoubleCircularBackwardIterator~T~
    GeneralIterator~Derived,T~ <|-- BinaryTreeForwardIterator~T~
    GeneralIterator~Derived,T~ <|-- BinaryTreeBackwardIterator~T~
```

> **Nota:** `DoubleLinkedListForwardIterator<T>` es un alias de
> `LinkedListForwardIterator<T>`, y `DoubleCircularForwardIterator<T>` un alias
> de `CircularForwardIterator<T>`: el recorrido forward de la lista doble y de
> la doble-circular es idéntico al de sus versiones simples, por lo que se
> reutiliza la implementación en lugar de duplicar clases. Los iteradores del
> árbol viven en `GeneralTraits.h` y sirven para cualquier árbol cuyos nodos
> hereden de `BinaryTreeNode`.

## 3 · Contenedores

```mermaid
classDiagram
    class Vector~Traits~ {
        -Node* m_data
        -size_t m_size, m_capacity
        -mutex m_mutex
        +push_back(const T&, Ref)
        +insert()
        +write() ostream&
        +read() istream&
        +call() / rcall() / FirstThat() / ApplyFunction()
    }
    class LinkedList~Traits~ {
        #NodePtr m_pRoot, m_pTail
        #Compare m_comp
        #bool m_circular
        #mutable mutex m_mutex
        +push_back() virtual
        +insert() : openRing → internalInsert → updateLinks
        +clear() virtual
        +updateLinks() virtual : reparacion tras cada mutacion
        +openRing() / closeRing()
        +operator<</operator>>
        +call() / FirstThat() / ApplyFunction()
        +GetRoot()
    }
    class CircularLinkedList~T~ {
        ctor : activa m_circular
    }
    class DoubleLinkedList~Traits~ {
        +updateLinks() : anillo + walk de m_pPrev
        +rbegin() / rend()
    }
    class DoubleLinkedCircularList~T~ {
        ctor : activa m_circular
    }
    class Heap~Traits~ {
        -Vector~Traits~ m_data
        -mutable mutex m_mutex
        +push_back() : HeapifyUp
        +pop() : HeapifyDown
        +operator[]() / empty() / size()
        +begin() / end() const
        +clear()
        +getMutex()
    }
    class MinHeap~T~ {
        orden fijo: MinHeapTraits
    }
    class MaxHeap~T~ {
        orden fijo: MaxHeapTraits
    }
    class BinaryTree~Traits~ {
        #NodePtr m_root
        #Compare m_comp
        #mutable mutex m_mutex
        +insert() : internal_insert recursivo (setea padre)
        +push_back() : adaptador de insert
        +clear() : post-order
        +operator=() copia y move
        +begin()/end() : in-order
        +rbegin()/rend() : in-order inverso
        +getMutex()
    }
    LinkedList~Traits~ <|-- CircularLinkedList~T~ : LinkedList~CircularTraits~T~~
    LinkedList~Traits~ <|-- DoubleLinkedList~Traits~ : DoubleLinkedListTraits
    DoubleLinkedList~Traits~ <|-- DoubleLinkedCircularList~T~ : DoubleLinkedList~DoubleCircularTraits~T~~
    Heap~Traits~ <|-- MinHeap~T~ : Heap~MinHeapTraits~T~~
    Heap~Traits~ <|-- MaxHeap~T~ : Heap~MaxHeapTraits~T~~
```

> **Nota:** la lista y el vector ya no definen `operator<`, `operator>>`,
> `call`, `FirstThat` ni `ApplyFunction`: todo viene de los mixins de
> `GeneralContainer.h`. Las derivadas solo heredan (estilo `MinHeap`) y
> `rcall` está disponible para las que tienen `rbegin` mediante el
> `::rcall` libre.

## 4 · Traits

```mermaid
classDiagram
    class DefaultTraits~T,Compare~ {
        +value_type = T
        +Compare
    }
    class AscendingTraits~T~ {
        +Compare = std::less~T~
    }
    class DescendingTraits~T~ {
        +Compare = std::greater~T~
    }
    class LinkedListAscTraits~T~ {
        +Node = LinkedListNode~T~
        +ForwardIterator = LinkedListForwardIterator~T~
    }
    class LinkedListDescTraits~T~ {
        +Node = LinkedListNode~T~
        +ForwardIterator = LinkedListForwardIterator~T~
    }
    class CircularTraits~T~ {
        +ForwardIterator = CircularForwardIterator~T~
    }
    class DoubleLinkedListTraits~T~ {
        +Node = DoubleLinkedListNode~T~
        +ForwardIterator = DoubleLinkedListForwardIterator~T~
        +BackwardIterator = DoubleLinkedListBackwardIterator~T~
    }
    class DoubleCircularTraits~T~ {
        +ForwardIterator = DoubleCircularForwardIterator~T~
        +BackwardIterator = DoubleCircularBackwardIterator~T~
    }
    class VectorAscTraits~T~ {
        +Node = GeneralNode~T~
        +ForwardIterator = VectorForwardIterator~T~
        +BackwardIterator = VectorBackwardIterator~T~
    }
    class VectorDescTraits~T~ {
        +Node = GeneralNode~T~
        +ForwardIterator = VectorForwardIterator~T~
        +BackwardIterator = VectorBackwardIterator~T~
    }
    class MinHeapTraits~T~ {
    }
    class MaxHeapTraits~T~ {
    }
    class BinaryTreeTraits~T~ {
        +Node = BinaryTreeNode~T~
        +ForwardIterator = BinaryTreeForwardIterator~T~
        +BackwardIterator = BinaryTreeBackwardIterator~T~
    }
    class BinaryTreeDescTraits~T~ {
        +Node = BinaryTreeNode~T~
        +ForwardIterator = BinaryTreeForwardIterator~T~
        +BackwardIterator = BinaryTreeBackwardIterator~T~
    }
    DefaultTraits~T,Compare~ <|-- AscendingTraits~T~
    DefaultTraits~T,Compare~ <|-- DescendingTraits~T~
    AscendingTraits~T~ <|-- LinkedListAscTraits~T~
    DescendingTraits~T~ <|-- LinkedListDescTraits~T~
    LinkedListAscTraits~T~ <|-- CircularTraits~T~
    LinkedListAscTraits~T~ <|-- DoubleLinkedListTraits~T~
    DoubleLinkedListTraits~T~ <|-- DoubleCircularTraits~T~
    AscendingTraits~T~ <|-- VectorAscTraits~T~
    DescendingTraits~T~ <|-- VectorDescTraits~T~
    VectorAscTraits~T~ <|-- MinHeapTraits~T~
    VectorDescTraits~T~ <|-- MaxHeapTraits~T~
    AscendingTraits~T~ <|-- BinaryTreeTraits~T~
    DescendingTraits~T~ <|-- BinaryTreeDescTraits~T~
```

## 5 · Mixins (GeneralContainer.h)

```mermaid
classDiagram
    class Multi~Derived~ {
        +hereda de: Mixins (variadico)
    }
    class Printable~Derived~ {
        +operator<<(ostream&, const Derived&)
    }
    class Readable~Derived~ {
        +operator>>(istream&, Derived&)
    }
    class Applicable~Derived~ {
        +ApplyFunction() / FirstThat()
        +call()
        +rcall() : requires rbegin
    }
    Printable~Derived~ <|-- Multi~Derived~
    Readable~Derived~ <|-- Multi~Derived~
    Applicable~Derived~ <|-- Multi~Derived~
```

> **Nota:** cada contenedor declara su propia cadena
> `Multi<Contenedor<Traits>, Printable, Readable, Applicable>` y las clases
> derivadas la heredan de su base. `::rcall` es libre y opera sobre el tipo
> real del contenedor, por lo que también sirve para derivadas con
> `rbegin` (DLL, DoubleLinkedCircularList, BinaryTree).

## 6 · Composición

```mermaid
classDiagram
    class Vector~VectorAscTraits~ {
        usa: GeneralNode
    }
    class LinkedList~LinkedListAscTraits~ {
        usa: LinkedListNode
    }
    class CircularLinkedList~CircularTraits~ {
        hereda de: LinkedList
        solo activa m_circular
        usa: CircularForwardIterator
    }
    class DoubleLinkedList~DoubleLinkedListTraits~ {
        hereda de: LinkedList
        usa: DoubleLinkedListNode
        repara: m_pPrev via updateLinks
    }
    class DoubleLinkedCircularList~DoubleCircularTraits~ {
        hereda de: DoubleLinkedList
        solo activa m_circular
        usa: iteradores circulares
    }
    class Heap~Traits~ {
        compone: Vector
    }
    class MinHeap~MinHeapTraits~ {
        hereda de: Heap
    }
    class MaxHeap~MaxHeapTraits~ {
        hereda de: Heap
    }
    class BinaryTree~BinaryTreeTraits~ {
        usa: BinaryTreeNode
    }
    class foreach_h {
        ::call
        ::ApplyFunction
        ::FirstThat
    }
    class GeneralContainer_h {
        Multi / Printable / Readable / Applicable / ::rcall
    }
    Vector~VectorAscTraits~ *-- GeneralNode~T~ : almacena
    LinkedList~LinkedListAscTraits~ *-- LinkedListNode~T~ : almacena
    CircularLinkedList~CircularTraits~ ..|> LinkedList~LinkedListAscTraits~
    DoubleLinkedList~DoubleLinkedListTraits~ ..|> LinkedList~LinkedListAscTraits~
    DoubleLinkedCircularList~DoubleCircularTraits~ ..|> DoubleLinkedList~DoubleLinkedListTraits~
    Heap~Traits~ *-- Vector~Traits~ : m_data
    MinHeap~MinHeapTraits~ ..|> Heap~Traits~
    MaxHeap~MaxHeapTraits~ ..|> Heap~Traits~
    BinaryTree~BinaryTreeTraits~ *-- BinaryTreeNode~T~ : almacena
    Vector~VectorAscTraits~ ..> foreach_h
    LinkedList~LinkedListAscTraits~ ..> foreach_h
    CircularLinkedList~CircularTraits~ ..> foreach_h
    DoubleLinkedList~DoubleLinkedListTraits~ ..> foreach_h
    DoubleLinkedCircularList~DoubleCircularTraits~ ..> foreach_h
    Heap~Traits~ ..> foreach_h
    BinaryTree~BinaryTreeTraits~ ..> foreach_h
    Vector~VectorAscTraits~ ..> GeneralContainer_h
    LinkedList~LinkedListAscTraits~ ..> GeneralContainer_h
    Heap~Traits~ ..> GeneralContainer_h
    BinaryTree~BinaryTreeTraits~ ..> GeneralContainer_h
```
