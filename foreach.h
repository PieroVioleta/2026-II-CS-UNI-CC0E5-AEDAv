#ifndef __FOREACH_H__
#define __FOREACH_H__

// Variadic template to allow passing additional arguments to the function
// template <typename Container, typename Func, typename... Args>
// Iterator Level #1
// void ApplyFunction(Container &container, Func func, Args... args) {
//     for (size_t i = 0; i < container.size(); ++i) {
//         func(container[i], args...);
//     }
// }

// Iterator Level #2
// template <typename Container, typename Func, typename... Args>
// void ApplyFunction(Container &container, Func func, Args... args) {
//     for (auto iter = container.begin(); iter != container.end(); ++iter) {
//         func(*iter, args...);
//     }
// }

// Iterator Level #3
template <typename Iterator, typename Func, typename... Args>
void ApplyFunction(Iterator begin, Iterator end, Func func, Args... args) {
    for (auto iter = begin; iter != end; ++iter)
        func(*iter, args...);
}

// // Iterator Level #3
// template <typename Container, typename Func, typename... Args>
// void ApplyFunction(Container &container, Func func, Args... args) {
//     ::ApplyFunction(container.begin(), container.end(), func, args...);
// }

// Iterator Level #4
template <typename Container, typename Func, typename... Args>
void ApplyFunction(Container &container, Func func, Args... args) {
    for (auto &v : container)
        func(v, args...);
}

#endif // __FOREACH_H__