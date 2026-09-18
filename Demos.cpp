
#include <iostream>
#include <fstream> // ofstream para escribir en archivo
#include <string>
#include <thread>
#include <vector>
#include "foreach.h"
#include "containers/vector.h"
#include "Demos.h"
using namespace std;

void AddOne(TX &value) {
    value += 1;
}

template <typename T>
void AddX(T &value, T x) {
    value += x;
}

void Square(TX &value) {
    value *= value;
}

const int NThreads = 5;

// Inserta los elementos de 'values' usando NThreads threads en paralelo,
// cada uno con un subconjunto entrelazado, sin ninguna sincronizacion,
// para evidenciar las race conditions de un Container (push_back) cuando
// se accede concurrentemente: el resultado es no determinista y puede
// incluso perder elementos o crashear.
// (recibe un vector, no un initializer_list, para poder pasarle miles de
// valores generados en un loop y asi hacer mas visible la race condition)
template <typename Container>
void ConcurrentInsert(Container &container,
                       const vector<typename Container::value_type> &values) {
    size_t n = values.size();
    vector<thread> workers;
    for (int t = 0; t < NThreads; ++t) {
        workers.emplace_back([&container, &values, n, t](){
            for (size_t i = t; i < n; i += NThreads)
                container.push_back(values[i]);
        });
    }

    for (auto &worker : workers)
        worker.join();
}

template <typename Container, typename Func, typename... Args>
void TestContainer(Container &container,
                    const vector<typename Container::value_type> &values,
                    const string &filename,
                    Func func, Args... args) {
    ConcurrentInsert(container, values);

    // Impresion usando write()
    cout << "Container using write(): ";
    container.write(cout);
    cout << endl;

    // Escritura hacia un archivo, en modo append para acumular cada estado
    // (el archivo se deja vacio una vez al inicio, ver DemoVector)
    ofstream of(filename, ios::app);
    container.write(of);
    of << endl;
    of.close();

    // Escritura en pantalla usando cout directamente (operator<<)
    cout << "Container using cout directly: ";
    cout << container << endl;

    // ApplyFunction sobre cada elemento
    ::ApplyFunction(container, func, args...);
    cout << "Container after ApplyFunction: ";
    cout << container << endl;
}

void DemoVector() {
    // Dejamos los archivos vacios para que TestContainer acumule (append)
    // el estado del container tras cada paso
    ofstream("vector.txt", ios::trunc).close();

    Vector<VectorAscTraits<TX>> vec;
    TestContainer(vec, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, "vector.txt", AddOne);
    TestContainer(vec, {}, "vector.txt", AddX<TX>, 7);
    TestContainer(vec, {}, "vector.txt", Square);

    ofstream("vector_str.txt", ios::trunc).close();
    Vector<VectorAscTraits<string>> strVec;
    TestContainer(strVec, {"Hello", "World"}, "vector_str.txt", AddX<string>, "!-X");
}

// Con pocos elementos la ventana de la race condition es demasiado chica
// para notarla a simple vista. Aqui insertamos muchos elementos (generados
// en un loop, no a mano) concurrentemente y comparamos cuantos deberian
// haber entrado contra cuantos entraron realmente: si NThreads > 1 y
// push_back no esta sincronizado, es muy probable perder inserciones
// (dos threads leen el mismo m_size, uno pisa al otro) e incluso crashear
// (dos threads compitiendo dentro de resize(), que hace new/delete).
void DemoRaceCondition() {
    const size_t N = 200000;

    vector<TX> values(N);
    for (size_t i = 0; i < N; ++i)
        values[i] = static_cast<TX>(i);

    Vector<VectorAscTraits<TX>> vec;
    ConcurrentInsert(vec, values);

    long long expectedSum = 0;
    for (TX v : values) expectedSum += v;

    long long actualSum = 0;
    for (size_t i = 0; i < vec.size(); ++i) actualSum += vec[i];

    cout << "DemoRaceCondition: se esperaban " << N << " elementos, "
         << "el Vector quedo con " << vec.size() << endl;
    cout << "  suma esperada = " << expectedSum
         << ", suma obtenida = " << actualSum << endl;

    if (vec.size() != N || actualSum != expectedSum)
        cout << "  *** Race condition detectada: se perdieron inserciones (push_back / resize sin sincronizar) ***" << endl;
    else
        cout << "  No se perdio ningun elemento en esta corrida "
             << "(la race sigue ahi: vuelve a correr el programa varias veces, "
             << "o compila con -fsanitize=thread para verla siempre)" << endl;
}