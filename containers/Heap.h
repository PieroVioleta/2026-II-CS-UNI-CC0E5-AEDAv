#ifndef __HEAP_H__
#define __HEAP_H__
#include "../types.h"
#include "../foreach.h"
#include "vector.h"
#include "GeneralStreamable.h"

template <typename Traits>
class Heap : public Multi<Heap<Traits>, Printable, Readable> {

public:
    using value_type        = typename Traits::value_type;
    using Node              = typename Traits::Node;
    using NodePtr           = Node *;
    using Compare           = typename Traits::Compare;
private:
    Vector<Traits> m_data;

public:
    decltype(auto) begin() const { return m_data.begin(); }
    decltype(auto) end()   const { return m_data.end();   }

    void push_back(const value_type &value, Ref ref) {
        m_data.push_back(value, ref);
        HeapifyUp(m_data.size() - 1);
    }

    void HeapifyUp(size_t index) {
        while (index > 0) {
            size_t parentIndex = (index - 1) / 2;
            if (Compare()(m_data[index].getValue(), m_data[parentIndex].getValue())) {
                std::swap(m_data[index], m_data[parentIndex]);
                index = parentIndex;
            } else {
                break;
            }
        }
    }
    void HeapifyDown(size_t index);
    Node pop() {
        if (m_data.empty()) return Node();
        std::swap(m_data[0], m_data[m_data.size() - 1]);
        Node top = m_data[m_data.size() - 1];
        m_data.pop_back();
        HeapifyDown(0);
        return top;
    }

    void clear() {
        m_data.clear();
    }

};

template <typename Traits>
class MinHeap : public Heap<Traits> {

};

template <typename Traits>
class MaxHeap : public Heap<Traits> {
    
};



#endif // __HEAP_H__