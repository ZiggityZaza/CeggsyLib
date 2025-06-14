#include "add.hpp"

int wrap_again() {
    Test::count++;
    return add(2, 4);
}