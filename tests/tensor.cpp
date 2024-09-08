#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <array>

#include "game/tensor.hpp"


using namespace game;


TEST_CASE("Shape") {

    tensor<char, -1> v(999);
    tensor<long, -1, 10> w(7);
    tensor<int, 1, 2, 3> x;
    tensor<float, -1, 10, -1, 20, -1> y(5, 6, 7);
    tensor<tensor<int, 2>, 0> z;

    CHECK(v.shape() == shape_t<999>());
    CHECK(w.shape() == shape_t<7, 10>());
    CHECK(x.shape() == shape_t<1, 2, 3>());
    CHECK(y.shape() == shape_t<5, 10, 6, 20, 7>());
    CHECK(z.shape() == shape_t<0>());

    CHECK(v.as_tensor().shape() == v.shape());
    CHECK(w.as_tensor().shape() == w.shape());
    CHECK(x.as_tensor().shape() == x.shape());
    CHECK(y.as_tensor().shape() == y.shape());
    CHECK(z.as_tensor().shape() == z.shape());

    CHECK(v.as_view().shape() == v.shape());
    CHECK(w.as_view().shape() == w.shape());
    CHECK(x.as_view().shape() == x.shape());
    CHECK(y.as_view().shape() == y.shape());
    CHECK(z.as_view().shape() == z.shape());

    CHECK(v.as_view().as_tensor().shape() == v.shape());
    CHECK(w.as_view().as_tensor().shape() == w.shape());
    CHECK(x.as_view().as_tensor().shape() == x.shape());
    CHECK(y.as_view().as_tensor().shape() == y.shape());
    CHECK(z.as_view().as_tensor().shape() == z.shape());

    CHECK(w[0].shape() == shape_t<10>());
    CHECK(x[0].shape() == shape_t<2, 3>());
    CHECK(y[0].shape() == shape_t<10, 6, 20, 7>());
}


TEST_CASE("Comparison") {

    tensor<float, 3> v = { 4.0f, 5.0f, 6.0f };
    tensor<float, 2, 3> w = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };

    CHECK(v == w[1]);
    CHECK(v == w[1].as_tensor());
    CHECK(v.as_view() == w[1].as_tensor());
    CHECK(v.as_view() == w[1]);

    CHECK(v != w[0]);
}


/*
TEST_CASE("Reshape") {

    tensor<int, -1> x;

    CHECK(x.shape() == std::array<dim_t, 1> { 0 });

    x = tensor<int, -1>(10);

    CHECK(x.shape() == std::array<dim_t, 1> { 10 });

    x.reshape({ 7 });

    CHECK(x.shape() == std::array<dim_t, 1> { 7 });

    tensor<int, -1, 2, -1> y;

    CHECK(y.shape() == std::array<dim_t, 3> { 0, 2, 0 });

    y = tensor<int, -1, 2, -1>({ 5, 8 });

    CHECK(y.shape() == std::array<dim_t, 3> { 5, 2, 8 });

    y.reshape({ 27, 2, 4 });

    CHECK(y.shape() == std::array<dim_t, 3> { 27, 2, 4 });
}
*/
