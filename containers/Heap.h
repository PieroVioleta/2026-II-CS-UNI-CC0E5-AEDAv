#ifndef __HEAP_H__
#define __HEAP_H__
#include "../types.h"
#include "../foreach.h"
#include "GeneralTraits.h"
#include "vector.h"
#include "GeneralContainer.h"

template <typename Traits>
class Heap : public Multi<Heap<Traits>, Printable, Readable, Applicable> {

public:
    using value_type        = typename Traits::value_type;
    using Node              = typename Traits::Node;
    using NodePtr           = Node *;
    using Compare           = typename Traits::Compare;
private:
    Vector<Traits> m_data;
    mutable std::mutex m_mutex;
    Compare m_comp;

    void HeapifyUp(size_t index) {
        while (index > 0) {
            size_t parentIndex = (index - 1) / 2;
            if (m_comp(m_data[index].getValue(), m_data[parentIndex].getValue())) {
                std::swap(m_data[index], m_data[parentIndex]);
                index = parentIndex;
            } else {
                break;
            }
        }
    }
    void HeapifyDown(size_t index) {
        size_t n = m_data.size();
        while (true) {
            size_t left = 2 * index + 1, right = 2 * index + 2, best = index;
            if (left < n && m_comp(m_data[left].getValue(), m_data[best].getValue()))
                best = left;
            if (right < n && m_comp(m_data[right].getValue(), m_data[best].getValue()))
                best = right;
            if (best == index)
                break;
            std::swap(m_data[index], m_data[best]);
            index = best;
        }
    }

public:
    decltype(auto) begin() const { return m_data.begin(); }
    decltype(auto) end()   const { return m_data.end();   }

    void push_back(const value_type &value, Ref ref) {
        lock_guard<mutex> lock(m_mutex);
        m_data.push_back(value, ref);
        HeapifyUp(m_data.size() - 1);
    }

    Node pop() {
        lock_guard<mutex> lock(m_mutex);
        if (m_data.empty()) return Node();
        std::swap(m_data[0], m_data[m_data.size() - 1]);
        Node top = m_data[m_data.size() - 1];
        m_data.pop_back();
        HeapifyDown(0);
        return top;
    }

    bool empty() const { return m_data.empty(); }
    size_t size() const { return m_data.size(); }
    Node &operator[](size_t index) { return m_data[index]; }

    void clear() {
        lock_guard<mutex> lock(m_mutex);
        m_data.clear();
    }

    mutex &getMutex() { return m_mutex; }

};

template <typename T>
class MinHeap : public Heap<MinHeapTraits<T>> {

};

template <typename T>
class MaxHeap : public Heap<MaxHeapTraits<T>> {

};



#endif // __HEAP_H__