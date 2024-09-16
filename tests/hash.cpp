#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <array>
#include <cmath>
#include <vector>

#include "game/hash.hpp"
#include "game/shape.hpp"
#include "game/tensor.hpp"


using namespace game;


TEST_CASE("Built-in types") {

    size_t a = 31;
    float b = 1.0f;
    double c = NAN;
    char d = 'A';
    int e = 18376;

    CHECK(hash_many() == hash_basis);
    CHECK(hash_many(a) == (31 ^ hash_basis) * hash_prime);
    CHECK(hash_many(a, b));
    CHECK(hash_many(a, b, c));
    CHECK(hash_many(a, b, c, d));
    CHECK(hash_many(a, b, c, d, e));
}


TEST_CASE("Collections") {

    tensor<int, 2, 3> x = {
        1, 2, 3,
        4, 5, 6,
    };

    std::array<float, 4> y = { 1.0f, 2.0f, 3.0f, 4.0f };

    std::vector<int64_t> z = { 1, 2, 3, 4, 5 };

    CHECK(hash_many(x));
    CHECK(hash_many(x.shape()));
    CHECK(hash_many(y));
    CHECK(hash_many(z));
}
