
#include <iostream>
#include <fstream> // ofstream para escribir en archivo
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "foreach.h"
#include "containers/vector.h"
#include "containers/LinkedList.h"
#include "containers/Heap.h"
#include "containers/CircularLinkedList.h"
#include "containers/DoubleLinkedList.h"
#include "containers/DoubleLinkedCircularList.h"
#include "containers/binarytree.h"
#include "Demos.h"
using namespace std;

void AddOne(GeneralNode<TX> &node) {
    node.value() += 1;
}

template <typename T>
void AddX(GeneralNode<T> &node, T x) {
    node.value() += x;
}

void Square(GeneralNode<TX> &node) {
    node.value() *= node.value();
}

// Predicado generico para FirstThat/call: true si el valor del nodo es
// mayor que 'threshold'. Generico en Node y T para servir tanto a
// Vector<VectorAscTraits<TX>> (int) como a Vector<VectorAscTraits<string>>.
template <typename Node, typename T>
bool IsGreaterThan(Node &node, T threshold) {
    return node.getValue() > threshold;
}

template <typename Node>
void PrintNode(Node &node, ostream &os) {
    os << node << " ";
}
const int NThreads = 5;

// Datos de ejemplo compartidos entre demos
static const vector<pair<TX, Ref>> Values0To4 = {{0, 10}, {1, 11}, {2, 12}, {3, 13}, {4, 14}};
static const vector<pair<TX, Ref>> ValuesHeap = {{5, 50}, {1, 10}, {8, 80}, {3, 30}, {9, 90}, {2, 20}, {7, 70}, {4, 40}, {6, 60}};

// Recorre el Container hacia atras (rbegin/rend) imprimiendo los nodos.
template <typename Container>
void TestBackward(Container &container) {
    cout << "Recorrido backward: ";
    for (auto it = container.rbegin(); it != container.rend(); ++it)
        cout << *it << " ";
    cout << endl;
}

// Serializa el Container en un stringstream y lo relee en una copia
// (operator<< / operator>>); imprime ambas versiones.
template <typename Container>
void TestPersist(Container &container, const string &name) {
    stringstream ss;
    ss << container;
    cout << name << " write: " << ss.str() << endl;
    Container copy;
    ss >> copy;
    cout << name << " read:  " << copy << endl;
}

// Aplica AddOne a todos los nodos e imprime el estado resultante.
template <typename Container>
void TestApplyAddOne(Container &container, const string &name) {
    container.ApplyFunction(AddOne);
    cout << name << " tras ApplyFunction(AddOne): " << container << endl;
}

// nThreads workers insertan concurrentemente n parejas por hilo: el hilo t
// mete el valor (t*n + i) y pusher decide el metodo de insercion (push_back
// o insert) y el ref asociado. Espera (join) a todos al terminar.
template <typename Container, typename Pusher>
void ConcurrentInsert(Container &container, int nThreads, int n, Pusher pusher) {
    vector<thread> workers;
    for (int t = 0; t < nThreads; ++t) {
        workers.emplace_back([&container, pusher, t, n]() {
            for (int i = 0; i < n; ++i)
                pusher(container, t * n + i);
        });
    }
    for (auto &worker : workers)
        worker.join();
}

// Inserta secuencialmente cada pareja (valor, ref) de 'values' en el
// Container, una por una. La insercion concurrente queda aislada en
// DemoRaceCondition(), que es donde se estudia esa problematica.
template <typename Container>
void InsertElements(Container &container,
                     const vector<pair<typename Container::value_type, Ref>> &values) {
    for (const auto &v : values)
        container.push_back(v.first, v.second);
}

template <typename Container>
void TestContainer(Container &container,
                    const vector<pair<typename Container::value_type, Ref>> &values,
                    const string &filename) {
    InsertElements(container, values);

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
}

// Prueba los recorridos del Container hacia adelante (begin/end) y hacia
// atras (rbegin/rend) solamente, imprimiendo los elementos en cada sentido.
template <typename Container>
void TestTraversal(Container &container) {
    using Node = typename Container::Node;

    cout << "Forward traversal:  [";
    container.call(PrintNode<Node>, cout);
    cout << "]" << endl;

    cout << "Backward traversal: [";
    container.rcall(PrintNode<Node>, cout);
    cout << "]" << endl;

    // Prueba de call()/FirstThat() con valor de retorno: busca el primer
    // nodo cuyo valor sea mayor que el del primer elemento del container.
    if (!container.empty()) {
        auto threshold = (*container.begin()).getValue();
        Node &found = container.FirstThat(IsGreaterThan<Node, decltype(threshold)>, threshold);
        cout << "FirstThat (primer valor > " << threshold << "): " << found << endl;
    }
}

// Prueba los recorridos hacia únicamente hacia adelante, especialmente para una LinkedList
template <typename Container>
void TestForwardTraversal(Container &container){
    using Node = typename Container::Node;

    cout << "Prueba de función call con std::cout para LinkedList [";
    container.call(PrintNode<Node>, cout);
    cout << "]" << endl;

    cout << "Prueba de uso de iteradores para impresión: [";
    for (auto &&node : container)
        cout << node << " ";
    cout << "]" << endl;

    // Prueba de FirstThat(): primer nodo cuyo valor sea mayor que el primero
    if (container.begin() != container.end()) {
        auto threshold = (*container.begin()).getValue();
        Node &found = container.FirstThat(IsGreaterThan<Node, decltype(threshold)>, threshold);
        cout << "FirstThat (primer valor > " << threshold << "): " << found << endl;
    }
}

void DemoVector() {
    // Dejamos los archivos vacios para que TestContainer acumule (append)
    // el estado del container tras cada paso
    ofstream("vector.txt", ios::trunc).close();

    // Cada elemento es una pareja (valor, ref) que se guarda en un Node;
    // el constructor initializer_list arma el Vector inicial de una vez
    Vector<VectorAscTraits<TX>> vec({{0, 10}, {1, 11}, {2, 12}, {3, 13}, {4, 14}});
    TestContainer(vec, {{5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}}, "vector.txt");
    TestTraversal(vec);

    ofstream("vector_str.txt", ios::trunc).close();
    Vector<VectorAscTraits<string>> strVec;
    TestContainer(strVec, {{"Hello", 1}, {"World", 2}}, "vector_str.txt");
    TestTraversal(strVec);
}

// Insertamos muchos elementos (generados en un loop, no a mano)
// concurrentemente y comparamos cuantos deberian haber entrado contra
// cuantos entraron realmente. Si push_back no estuviera sincronizado
// (sin el mutex/lock_guard actual) esto perderia inserciones o crashearia;
// con el mutex protegiendo push_back/resize, deberia dar siempre 0 perdidas.
void DemoRaceCondition() {
    const size_t N = 200000;

    vector<pair<TX, Ref>> values(N);
    for (size_t i = 0; i < N; ++i)
        values[i] = {static_cast<TX>(i), static_cast<Ref>(i)};

    Vector<VectorAscTraits<TX>> vec;

    // Insercion concurrente: NThreads workers insertando en paralelo sobre
    // el mismo Vector, cada uno con un subconjunto entrelazado (stride)
    size_t n = values.size();
    vector<thread> workers;
    for (int t = 0; t < NThreads; ++t) {
        workers.emplace_back([&vec, &values, n, t](){
            for (size_t i = t; i < n; i += NThreads)
                vec.push_back(values[i].first, values[i].second);
        });
    }
    for (auto &worker : workers)
        worker.join();

    long long expectedSum = 0;
    for (auto &v : values) expectedSum += v.first;

    long long actualSum = 0;
    for (size_t i = 0; i < vec.size(); ++i) actualSum += vec[i].getValue();

    cout << "DemoRaceCondition: se esperaban " << N << " elementos, "
         << "el Vector quedo con " << vec.size() << endl;
    cout << "  suma esperada = " << expectedSum
         << ", suma obtenida = " << actualSum << endl;

    if (vec.size() != N || actualSum != expectedSum)
        cout << "  *** Race condition detectada: se perdieron inserciones (push_back / resize sin sincronizar) ***" << endl;
    else
        cout << "  No se perdio ningun elemento: el mutex de push_back/resize "
             << "sincroniza correctamente las inserciones concurrentes" << endl;
}

// TODO: Implementar DemoLinkedList() para probar la lista enlazada y sus iteradores.
void DemoLinkedList()
{
    using IntLinkedList = LinkedList<LinkedListAscTraits<TX>>;

    ofstream("linkedlist.txt", ios::trunc).close();

    // Pruebas de pushback
    IntLinkedList list;
    TestContainer(list, {{5, 15}, {1, 11}, {8, 18}, {3, 13}}, "linkedlist.txt");
    TestForwardTraversal(list);

    InsertElements(list, Values0To4);

    // Pruebas de insert
    list.insert(4, 12);
    list.insert(11, 20);
    list.insert(7, 6);
    cout << "LinkedList luego de hacer 3 inserts " << list << endl;

    // Prueba de apply function
    list.ApplyFunction(AddOne);
    cout << "LinkedList tras usar Add One a sus elementos: " << list << endl;
    list.ApplyFunction(AddX<TX>, TX(10));
    cout << "LinkedList tras usar AddX(10) a sus elementos: " << list << endl;

    // Prueba de clear
    list.clear();
    cout << "LinkedList luego de usar clear" << list << endl;
    list.push_back(10, 2);
    cout << "LinkedList luego de un nuevo push_back " << list << endl;

    // Prueba de lectura
    IntLinkedList new_list;
    ifstream in("linkedlist.txt");
    in >> new_list;
    in.close();
    cout << "LinkedList leida desde archivo: " << new_list << endl;
    TestForwardTraversal(new_list);
}

void DemoHeap()
{
    // uso
    MinHeap<TX> minHeap;
    MaxHeap<TX> maxHeap;
    InsertElements(minHeap, ValuesHeap);
    InsertElements(maxHeap, ValuesHeap);
    cout << "MinHeap (raiz = menor): " << minHeap << endl;
    cout << "MaxHeap (raiz = mayor): " << maxHeap << endl;

    cout << "Recorrido MinHeap: ";
    minHeap.call(PrintNode<MinHeap<TX>::Node>, cout);
    cout << endl;

    TestApplyAddOne(minHeap, "MinHeap");
    TestApplyAddOne(maxHeap, "MaxHeap");

    // persistencia
    TestPersist(minHeap, "MinHeap");

    // concurrencia
    MinHeap<TX> concurrente;
    const int NThreads = 4, N = 250;
    ConcurrentInsert(concurrente, NThreads, N, [](auto &c, TX v) { c.push_back(v, v * 10); });

    bool ok = true;
    for (size_t i = 1; i < concurrente.size() && ok; ++i)
        if (concurrente[i].getValue() < concurrente[(i - 1) / 2].getValue())
            ok = false;

    long long suma = 0, esperada = 0;
    while (!concurrente.empty()) {
        suma += concurrente.pop().getValue();
    }
    for (int v = 0; v < NThreads * N; ++v)
        esperada += v;

    cout << "MinHeap concurrencia: " << NThreads * N << " inserciones, propiedad heap "
         << (ok ? "OK" : "ROTO") << ", suma " << (suma == esperada ? "integra" : "PERDIDA") << endl;
}

void DemoCircularLinkedList()
{
    // uso
    CircularLinkedList<TX> list;
    list.push_back(3, 30);
    list.push_back(1, 10);
    list.push_back(2, 20);
    cout << "CircularLinkedList luego de push_back x3: " << list << endl;

    list.insert(2, 25);
    cout << "CircularLinkedList luego de insert(2,25): " << list << endl;

    // bucle nativo
    cout << "Bucle nativo sobre el anillo: ";
    LinkedListNode<TX> *p = list.GetRoot();
    do {
        cout << *p << " ";
        p = p->m_pNext;
    } while (p != list.GetRoot());
    cout << endl;

    // persistencia
    TestPersist(list, "CircularLinkedList");

    // call
    TestApplyAddOne(list, "CircularLinkedList");

    list.call(AddX<TX>, TX(10));
    cout << "CircularLinkedList tras call(AddX, 10):        " << list << endl;

    cout << "CircularLinkedList FirstThat(>5): " << list.FirstThat([](GeneralNode<TX> &n){ return n.getValue() > 5; }) << endl;

    try {
        list.FirstThat([](GeneralNode<TX> &n){ return n.getValue() > 100; });
    } catch (const runtime_error &e) {
        cout << "CircularLinkedList FirstThat sin match: " << e.what() << endl;
    }

    // concurrencia
    list.clear();
    const int NThreads = 4;
    ConcurrentInsert(list, NThreads, 100, [](auto &c, TX v) { c.push_back(v, v * 10); });

    int count = 0;
    p = list.GetRoot();
    do {
        ++count;
        p = p->m_pNext;
    } while (p != list.GetRoot());

    cout << "CircularLinkedList concurrencia: " << NThreads * 100 << " inserciones, el anillo quedo con "
         << count << " nodos" << (count == NThreads * 100 ? " (integro)" : " (ROTO)") << endl;
}

void DemoDoubleLinkedList()
{
    // uso
    DoubleLinkedList<DoubleLinkedListTraits<TX>> list;
    list.push_back(3, 30);
    list.push_back(1, 10);
    list.push_back(2, 20);
    cout << "DoubleLinkedList luego de push_back x3: " << list << endl;

    list.insert(0, 5);
    cout << "DoubleLinkedList luego de insert(0,5):  " << list << endl;

    // backward
    TestBackward(list);

    rcall(list, AddX<TX>, TX(10));
    cout << "DoubleLinkedList tras rcall(AddX, 10):  " << list << endl;
}

void DemoDoubleCircularLinkedList()
{
    // uso
    DoubleLinkedCircularList<TX> list;
    list.push_back(3, 30);
    list.push_back(1, 10);
    list.push_back(2, 20);
    list.insert(0, 5);
    cout << "DoubleLinkedCircularList luego de push_back e insert: " << list << endl;

    // persistencia
    TestPersist(list, "DoubleLinkedCircularList");

    // backward
    TestBackward(list);

    rcall(list, AddX<TX>, TX(10));
    cout << "DoubleLinkedCircularList tras rcall(AddX, 10):       " << list << endl;
}

void DemoBinaryTree()
{
    // uso
    BinaryTree<BinaryTreeTraits<TX>> tree;
    tree.insert(5, 50);
    tree.insert(1, 10);
    tree.insert(8, 80);
    tree.insert(3, 30);
    tree.insert(9, 90);
    tree.insert(2, 20);
    tree.insert(7, 70);
    cout << "BinaryTree (in-order, ordenado): " << tree << endl;

    // bucle con iteradores
    cout << "Recorrido in-order: ";
    for (auto it = tree.begin(); it != tree.end(); ++it)
        cout << *it << " ";
    cout << endl;

    // backward
    TestBackward(tree);

    // persistencia
    TestPersist(tree, "BinaryTree");

    // call
    TestApplyAddOne(tree, "BinaryTree");

    tree.call(AddX<TX>, TX(10));
    cout << "BinaryTree tras call(AddX, 10):        " << tree << endl;

    rcall(tree, Square);
    cout << "BinaryTree tras rcall(Square):         " << tree << endl;

    cout << "BinaryTree FirstThat(>50): " << tree.FirstThat([](GeneralNode<TX> &n){ return n.getValue() > 50; }) << endl;

    // concurrencia
    tree.clear();
    const int NThreads = 4;
    ConcurrentInsert(tree, NThreads, 100, [](auto &c, TX v) { c.insert(v, v * 10); });

    size_t count = 0;
    auto it = tree.begin();
    while (it != tree.end()) { ++count; ++it; }
    cout << "BinaryTree concurrencia: " << NThreads * 100 << " inserciones, in-order "
         << (count == size_t(NThreads * 100) ? "integro" : "ROTO") << " (" << count << " nodos)" << endl;
}
