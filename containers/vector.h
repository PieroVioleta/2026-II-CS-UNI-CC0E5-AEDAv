#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "GeneralIterator.h"
using namespace std;
template <typename T>
class VectorForwardIterator : public GeneralIterator<VectorForwardIterator<T>> {
public:
    using value_type        = typename T;
    using MySelf            = VectorForwardIterator<T>;
    using Parent            = GeneralIterator<MySelf>;
    using Parent::Parent; // Inherit constructor
 
    // Prefix increment
    VectorForwardIterator& operator++() { ++Parent::m_ptr; return *this; }  
};

template <typename T>
struct VectorAscTraits {
    using value_type        = T;
    using ForwardIterator   = VectorForwardIterator<T>;
};

template <typename Traits>
class Vector {
    using value_type        = Traits::value_type;
    using ForwardIterator   = Traits::ForwardIterator;
private:
    value_type  *m_data     = nullptr;   // puntero al arreglo dinámico
    size_t       m_size     = 0,         // cantidad actual
                 m_capacity = 0;         // capacidad

    void resize(size_t new_cap) {
        if (new_cap <= m_capacity) return;
        value_type* new_data = new value_type[new_cap];
        for (size_t i = 0; i < m_size; ++i)
            new_data[i] = m_data[i];
        delete[] m_data;
        m_data = new_data;
        m_capacity = new_cap;
    }

public:
    Vector() {}

    virtual ~Vector() {
        clear();
    }

    Vector(const Vector& other) {
        resize(other.m_size);
        for (size_t i = 0; i < other.m_size; ++i)
            m_data[i] = other.m_data[i];
        m_size = other.m_size;
    }

    // Move constructor and move assignment operator
    Vector(Vector&& other) noexcept {
        m_data     = std::exchange(other.m_data, nullptr);
        m_size     = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);
    }

    // Move assignment operator
    Vector& operator=(Vector&& other) noexcept {
        m_data     = std::exchange(other.m_data, nullptr);
        m_size     = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);
        return *this;
    }

    void push_back(const value_type& value) {
        if (m_size == m_capacity) {
            size_t new_cap = (m_capacity == 0) ? 1 : m_capacity * 2;
            resize(new_cap);
        }
        m_data[m_size] = value;
        ++m_size;
    }

    void pop_back() {
        if (m_size > 0) {
            --m_size;
        }
    }

    value_type& operator[](size_t index) {
        if (index >= m_size) throw std::out_of_range("Indice fuera de rango");
        return m_data[index];
    }

    value_type& at(size_t index) {
        if (index >= m_size) throw std::out_of_range("Indice fuera de rango");
        return m_data[index];
    }

    size_t size()     const { return m_size; }
    size_t capacity() const { return m_capacity; }
    bool   empty()    const { return m_size == 0; }

    void clear() {
        delete [] m_data;
        m_data     = nullptr;
        m_size     = 0;
        m_capacity = 0;
    }

    ForwardIterator begin() { return ForwardIterator(m_data); }
    ForwardIterator end()   { return ForwardIterator(m_data + m_size); }

    // Persistencia
    ostream &write(ostream &os){
        os << "[";
        for (size_t i = 0; i < size()-1; ++i)
            os << m_data[i] << " ";
        if (size() > 0)
            os << m_data[size()-1];
        return os << "]";
    }

    // TODO: implementar la lectura de un vector desde un stream
    istream &read(istream &is){
        // Implementation for reading vector from stream
    }
    // TODO: aplicarle una funcion a cada elemento.
    //       ej. sumarle un valor x
    // Variadic template to allow passing additional arguments to the function
    // Iterator Level #0
    template <typename Func, typename... Args>
    void ApplyFunction(Func func, Args... args) {
        for (size_t i = 0; i < size(); ++i) {
            func(m_data[i], args...);
        }
    }
};

template <typename T>
ostream& operator<<(ostream &os, Vector<T> &vec) {
    return vec.write(os);
}

template <typename T>
istream& operator>>(istream &is, Vector<T> &vec) {
    return vec.read(is);
}

#endif // __VECTOR_H__