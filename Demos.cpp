
#include <iostream>
#include <fstream> // ofstream para escribir en archivo
#include <string>
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

void DemoVector() {
    Vector<VectorAscTraits<TX>> vec;
    for (TX i = 0; i < 10; ++i) {
        vec.push_back(i);
    }

    cout << "Vector using print: ";
    vec.write(cout);
    cout << endl;

    ofstream of("vector.txt");
    vec.write(of);
    of.close();

    cout << "Vector using cout directly: ";
    cout << vec << endl;

    ::ApplyFunction(vec, AddOne); 
    // vec.ApplyFunction(AddOne);
    cout << "Vector after applying AddOne function: ";
    cout << vec << endl;

    ::ApplyFunction(vec, AddX<TX>, 7);
    //vec.ApplyFunction(AddX<TX>, 5);
    cout << "Vector after applying lambda function (Add 5): ";
    cout << vec << endl;

    ::ApplyFunction(vec, Square);
    // vec.ApplyFunction(Square);
    cout << "Vector after applying Square function: ";
    cout << vec << endl;

    Vector<VectorAscTraits<string>> strVec;
    strVec.push_back("Hello");
    strVec.push_back("World");

    cout << "VectorStr: ";
    cout << strVec << endl;

    ::ApplyFunction(strVec, AddX<string>, "!-X");
    // strVec.ApplyFunction(AddX<string>, "!");
    cout << "VectorStr after applying lambda function (Add !): ";
    cout << strVec << endl;


}