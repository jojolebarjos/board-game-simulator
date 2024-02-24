#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <array>
#include <cmath>

#include "game/hash.hpp"
#include "game/tensor.hpp"


using namespace game;


TEST_CASE("Built-in types") {

    size_t a = 31;
    float b = 1.0f;
    double c = NAN;
    char d = 'A';
    int e = 18376;

    CHECK(hash() == HASH_BASIS);
    CHECK(hash(a) == (HASH_BASIS ^ 31) * HASH_PRIME);
    CHECK(hash(a, b));
    CHECK(hash(a, b, c));
    CHECK(hash(a, b, c, d));
    CHECK(hash(a, b, c, d, e));
}


TEST_CASE("Collections") {

    tensor<int, 2, 3> x = {
        1, 2, 3,
        4, 5, 6,
    };

    CHECK(hash(x));
}
