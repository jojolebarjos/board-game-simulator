#ifndef GAME_SHAPE_HPP
#define GAME_SHAPE_HPP


#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>

#include "./hash.hpp"


namespace game {


// TODO maybe use int64_t, like in nb::ndarray?
typedef int dim_t;


/*
 * Let us use a custom exception for all shape-related errors, which typically
 * occur when using dynamic shapes.
 */
struct shape_error : std::runtime_error {
    shape_error() : std::runtime_error("invalid shape") {}
};


template <dim_t... N>
struct shape_t {

    static constexpr size_t ndim = 0;

    constexpr size_t size() const {
        return ndim;
    }

    constexpr size_t product() const {
        return 1;
    }

    constexpr dim_t operator[](size_t i) const {
        return 0;
    }

    constexpr std::array<dim_t, ndim> to_array() const {
        return {};
    }

    constexpr bool from_array(dim_t const* data, size_t size) {
        return size == 0;
    }

    constexpr bool from_array(std::array<dim_t, ndim> const&) {
        return true;
    }
};


template <dim_t Head, dim_t... Tail>
struct shape_t<Head, Tail...> {

    static constexpr size_t ndim = 1 + sizeof...(Tail);

    constexpr size_t size() const {
        return ndim;
    }

    constexpr size_t product() const {
        return (Head * ... * Tail);
    }

    constexpr dim_t operator[](size_t i) const {
        return i == 0 ? Head : tail()[i - 1];
    }

    constexpr std::array<dim_t, ndim> to_array() const {
        return { Head, Tail... };
    }

    constexpr bool from_array(dim_t const* data, size_t size) {
        if (size != ndim || data[0] != Head)
            return false;
        return tail().from_array(data + 1, size - 1);
    }

    constexpr bool from_array(std::array<dim_t, ndim> const& array) {
        return from_array(array.data(), array.size());
    }

    constexpr dim_t head() const {
        return Head;
    }

    constexpr shape_t<Tail...> tail() const {
        return {};
    }
};


template <dim_t... Tail>
struct shape_t<-1, Tail...> {

    dim_t h;

    static constexpr size_t ndim = 1 + sizeof...(Tail);

    constexpr size_t size() const {
        return ndim;
    }

    constexpr size_t product() const {
        return (h * ... * Tail);
    }

    constexpr dim_t operator[](size_t i) const {
        return i == 0 ? h : tail()[i - 1];
    }

    constexpr std::array<dim_t, ndim> to_array() const {
        return { h, Tail... };
    }

    constexpr bool from_array(dim_t const* data, size_t size) {
        if (size != ndim)
            return false;
        if (tail().from_array(data + 1, size - 1)) {
            h = data[0];
            return true;
        }
        return false;
    }

    constexpr bool from_array(std::array<dim_t, ndim> const& array) {
        return from_array(array.data(), array.size());
    }

    constexpr dim_t head() const {
        return h;
    }

    constexpr shape_t<Tail...> tail() const {
        return {};
    }
};


template <dim_t Head, dim_t... Tail>
    requires ((Tail < 0) || ...)
struct shape_t<Head, Tail...> {

    shape_t<Tail...> t;

    static constexpr size_t ndim = 1 + sizeof...(Tail);

    constexpr size_t size() const {
        return ndim;
    }

    constexpr size_t product() const {
        return Head * t.product();
    }

    constexpr dim_t operator[](size_t i) const {
        return i == 0 ? Head : t[i - 1];
    }

    constexpr std::array<dim_t, ndim> to_array() const {
        // TODO can we do better than this?
        std::array<dim_t, ndim> a = { Head };
        std::array<dim_t, ndim - 1> b = t.to_array();
        std::copy_n(b.data(), ndim - 1, a.data() + 1);
        return a;
    }

    constexpr bool from_array(dim_t const* data, size_t size) {
        if (size != ndim || data[0] != Head)
            return false;
        return t.from_array(data + 1, size - 1);
    }

    constexpr bool from_array(std::array<dim_t, ndim> const& array) {
        return from_array(array.data(), array.size());
    }

    constexpr dim_t head() const {
        return Head;
    }

    constexpr shape_t<Tail...> tail() const {
        return t;
    }
};


template <dim_t... Tail>
    requires ((Tail < 0) || ...)
struct shape_t<-1, Tail...> {

    dim_t h;
    shape_t<Tail...> t;

    static constexpr size_t ndim = 1 + sizeof...(Tail);

    constexpr size_t size() const {
        return ndim;
    }

    constexpr size_t product() const {
        return h * t.product();
    }

    constexpr dim_t operator[](size_t i) const {
        return i == 0 ? h : t[i - 1];
    }

    constexpr std::array<dim_t, ndim> to_array() const {
        // TODO can we do better than this?
        std::array<dim_t, ndim> a = { h };
        std::array<dim_t, ndim - 1> b = t.to_array();
        std::copy_n(b.data(), ndim - 1, a.data() + 1);
        return a;
    }

    constexpr bool from_array(dim_t const* data, size_t size) {
        if (size != ndim)
            return false;
        if (t.from_array(data + 1, size - 1)) {
            h = data[0];
            return true;
        }
        return false;
    }

    constexpr bool from_array(std::array<dim_t, ndim> const& array) {
        return from_array(array.data(), array.size());
    }

    constexpr dim_t head() const {
        return h;
    }

    constexpr shape_t<Tail...> tail() const {
        return t;
    }
};


template <dim_t... L, dim_t... R>
constexpr auto operator<=>(shape_t<L...> const& left, shape_t<R...> const& right) {
    auto l = left.to_array();
    auto r = right.to_array();
    return std::lexicographical_compare_three_way(l.begin(), l.end(), r.begin(), r.end());
}


template <dim_t... L, dim_t... R>
constexpr bool operator==(shape_t<L...> const& left, shape_t<R...> const& right) {
    if constexpr (shape_t<L...>::ndim != shape_t<R...>::ndim) {
        return false;
    }
    else {
        return left.to_array() == right.to_array();
    }
}


template <dim_t... N>
struct hash<shape_t<N...>> {
    constexpr size_t operator()(shape_t<N...> const& value) const {
        return hash_value(value.to_array());
    }
};


}


#endif
