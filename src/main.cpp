#include <any>
#include <iostream>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <cstring>
#include "any.hpp"

int main() {
    Any a{10};
    {
        Any b{std::move(a)};
    }
    return 0;
}