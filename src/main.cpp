#include <iostream>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <cstring>
#include "any.hpp"

int main() {
    Any any;
    any.emplace<double>(14.88);
    Any any2{std::move(any)};
    
    std::cout << "'" << anycast<double>(any2) << "'" << std::endl;
    
    return 0;
}