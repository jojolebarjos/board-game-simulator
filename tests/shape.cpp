#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <array>

#include "game/shape.hpp"


using namespace game;


TEST_CASE("To array") {

    shape_t<2, 3, 4> a = {};
    shape_t<-1, 10, -1, 20> b = { 5, 15 };
    shape_t<7> c = {};
    shape_t<-1> d = { 1000 };

    CHECK(a.to_array() == std::array<dim_t, 3> { 2, 3, 4 });
    CHECK(b.to_array() == std::array<dim_t, 4> { 5, 10, 15, 20 });
    CHECK(c.to_array() == std::array<dim_t, 1> { 7 });
    CHECK(d.to_array() == std::array<dim_t, 1> { 1000 });
}


TEST_CASE("From array") {

    shape_t<2, 3, 4> a = {};
    shape_t<-1, 10, -1, 20> b = { 5, 15 };
    shape_t<7> c = {};
    shape_t<-1> d = { 1000 };

    std::array<dim_t, 3> m = { 2, 3, 4 };
    CHECK(a.from_array(m));

    std::array<dim_t, 3> n = { 2, 999, 4 };
    CHECK(!a.from_array(n));

    std::array<dim_t, 4> o = { 1, 10, 2, 20 };
    CHECK(b.from_array(o));
    CHECK(b[0] == 1);
    CHECK(b[2] == 2);

    std::array<dim_t, 4> p = { 1, 10, 2, 999 };
    CHECK(!b.from_array(p));
    CHECK(b[0] == 1);
    CHECK(b[2] == 2);

    std::array<dim_t, 1> q = { 7 };
    CHECK(c.from_array(q));

    std::array<dim_t, 1> r = { 0 };
    CHECK(!c.from_array(r));

    std::array<dim_t, 1> s = { 42 };
    CHECK(d.from_array(s));
    CHECK(d[0] == 42);
}


TEST_CASE("Comparison") {

    shape_t<2, 3, 4> a = {};
    shape_t<-1, 3, -1> b = { 2, 4 };
    shape_t<2> c = {};
    shape_t<-1> d = { 4 };

    CHECK(a == b);
    CHECK(a > c);
    CHECK(c <= d);
    CHECK(b >= c);
    CHECK(a < d);
    CHECK(b != c);
    CHECK(a.tail().tail() == d);
}


TEST_CASE("Product") {

    shape_t<2, 3, 4> a = {};
    shape_t<-1, 10, -1, 20, -1> b = { 5, 15, 25 };
    shape_t<7> c = {};
    shape_t<-1> d = { 1000 };
    shape_t<> e = {};

    CHECK(a.product() == 24);
    CHECK(b.product() == 375000);
    CHECK(c.product() == 7);
    CHECK(d.product() == 1000);
    CHECK(e.product() == 1);
}
