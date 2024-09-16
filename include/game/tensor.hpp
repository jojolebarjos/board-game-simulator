#ifndef GAME_TENSOR_HPP
#define GAME_TENSOR_HPP


#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include "./shape.hpp"


namespace game {


/*
 * A multi-dimensional tensor, which is essentially a dense storage with an
 * associated shape.
 *
 * As much as possible, compile-time shape should be used. Dynamic shapes are
 * supported, using the placeholder size value -1 for one or more dimensions.
 *
 * Note that stride and other forms of non-dense storage are out-of-scope of
 * this implementation.
 */
template <typename T, dim_t Head, dim_t... Tail>
struct tensor;


/*
 * In some cases, for instance when working with tensor slices, an external
 * storage may be handy. This is called a view on another storage.
 *
 * Note that, while it is primary used in some tensor operations, it is
 * perfectly valid to be used on arbitrary memory location.
 */
template <typename T, dim_t Head, dim_t... Tail>
struct view;


/*
 * A fixed-shape tensor.
 */
template <typename T, dim_t Head, dim_t... Tail>
struct tensor {

    static constexpr unsigned ndim = 1 + sizeof...(Tail);
    static constexpr unsigned nddim = 0;

    std::array<T, (Head * ... * Tail)> storage;

    constexpr tensor() = default;

    constexpr explicit tensor(shape_t<Head, Tail...> const&) {}

    constexpr tensor(std::initializer_list<T> list) {
        if (list.size() != storage.size())
            throw std::invalid_argument("initializer list size mismatch");
        std::copy_n(list.begin(), storage.size(), storage.begin());
    }

    constexpr T* data() noexcept {
        return storage.data();
    }

    constexpr T const* data() const noexcept {
        return storage.data();
    }

    constexpr size_t size() const noexcept {
        return storage.size();
    }

    constexpr shape_t<Head, Tail...> shape() const noexcept {
        return {};
    }

    constexpr void reshape(shape_t<Head, Tail...> const&) {}

    constexpr view<T, Tail...> operator[](dim_t index) {
        return view<T, Tail...>(storage.data() + index * (Tail * ...));
    }

    constexpr view<T const, Tail...> operator[](dim_t index) const {
        return view<T const, Tail...>(storage.data() + index * (Tail * ...));
    }

    // TODO at
    // TODO operator()

    // TODO operator=

    constexpr void fill(T const& value) {
        storage.fill(value);
    }

    constexpr tensor<T, Head, Tail...> const& as_tensor() const {
        return *this;
    }

    constexpr view<T, Head, Tail...> as_view() {
        return view<T, Head, Tail...>(storage.data());
    }

    constexpr view<T const, Head, Tail...> as_view() const {
        return view<T const, Head, Tail...>(storage.data());
    }
};


/*
 * A dynamic-shaped tensor, where variable-size dimensions are explicitly
 * stored.
 */
template <typename T, dim_t Head, dim_t... Tail>
    requires ((Head < 0) || ... || (Tail < 0))
struct tensor<T, Head, Tail...> {

    static constexpr unsigned ndim = 1 + sizeof...(Tail);
    static constexpr unsigned nddim = ((Head < 0) + ... + (Tail < 0));

    std::vector<T> storage;
    shape_t<Head, Tail...> shape_;

    constexpr tensor() : shape_{ 0 } {}

    constexpr explicit tensor(shape_t<Head, Tail...> const& shape) : shape_(shape) {
        storage.resize(shape_.product());
    }

    template <typename... S>
    constexpr explicit tensor(dim_t h, S... t) : tensor(shape_t<Head, Tail...>{ h, dim_t(t)... }) {}

    constexpr T* data() noexcept {
        return storage.data();
    }

    constexpr T const* data() const noexcept {
        return storage.data();
    }

    constexpr size_t size() const noexcept {
        return storage.size();
    }

    constexpr shape_t<Head, Tail...> shape() const {
        return shape_;
    }

    constexpr void reshape(shape_t<Head, Tail...> const& s) {
        storage.resize(s.product());
        shape_ = s;
    }

    constexpr view<T, Tail...> operator[](dim_t index) {
        return { data() + index * shape_.tail().product(), shape_.tail()};
    }

    constexpr view<T const, Tail...> operator[](dim_t index) const {
        return { data() + index * shape_.tail().product(), shape_.tail()};
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        std::fill_n(storage.data(), storage.size(), value);
    }

    constexpr tensor<T, Head, Tail...> const& as_tensor() const {
        return *this;
    }

    constexpr view<T, Head, Tail...> as_view() {
        return view<T, Head, Tail...>(storage.data(), shape_);
    }

    constexpr view<T const, Head, Tail...> as_view() const {
        return view<T const, Head, Tail...>(storage.data(), shape_);
    }
};


/*
 * One-dimensional specialization, as the accessor must return a reference to a
 * scalar.
 */
template <typename T, dim_t Head>
struct tensor<T, Head> {

    static constexpr unsigned ndim = 1;
    static constexpr unsigned nddim = 0;

    std::array<T, Head> storage;

    constexpr tensor() = default;

    constexpr explicit tensor(shape_t<Head> const&) {}

    constexpr tensor(std::initializer_list<T> list) {
        if (list.size() != storage.size())
            throw std::invalid_argument("initializer list size mismatch");
        std::copy_n(list.begin(), storage.size(), storage.begin());
    }

    constexpr T* data() {
        return storage.data();
    }

    constexpr T const* data() const noexcept {
        return storage.data();
    }

    constexpr size_t size() const noexcept {
        return storage.size();
    }

    constexpr shape_t<Head> shape() const noexcept {
        return {};
    }

    constexpr void reshape(shape_t<Head> const&) {}

    constexpr T& operator[](dim_t index) {
        return storage[index];
    }

    constexpr T const& operator[](dim_t index) const {
        return storage[index];
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        storage.fill(value);
    }

    constexpr tensor<T, Head> const& as_tensor() const {
        return *this;
    }

    constexpr view<T, Head> as_view() {
        return view<T, Head>(storage.data());
    }

    constexpr view<T const, Head> as_view() const {
        return view<T const, Head>(storage.data());
    }
};


/*
 * Finally, the dynamic variant of a one-dimensional tensor.
 */
template <typename T>
struct tensor<T, -1> {

    static constexpr unsigned ndim = 1;
    static constexpr unsigned nddim = 1;

    std::vector<T> storage;

    constexpr tensor() = default;

    constexpr explicit tensor(shape_t<-1> const& shape) : storage(shape.head()) {}

    constexpr explicit tensor(dim_t h) : tensor(shape_t<-1>{ h }) {}

    constexpr explicit tensor(std::initializer_list<T> list) : storage(list) {}

    constexpr T* data() {
        return storage.data();
    }

    constexpr T const* data() const noexcept {
        return storage.data();
    }

    constexpr size_t size() const noexcept {
        return storage.size();
    }

    constexpr shape_t<-1> shape() const noexcept {
        return { (dim_t)storage.size() };
    }

    constexpr void reshape(shape_t<-1> const& s) {
        storage.resize(s.product());
    }

    constexpr T& operator[](dim_t index) {
        return storage[index];
    }

    constexpr T const& operator[](dim_t index) const {
        return storage[index];
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        std::fill_n(storage.data(), storage.size(), value);
    }

    constexpr tensor<T, -1> const& as_tensor() const {
        return *this;
    }

    constexpr view<T, -1> as_view() {
        return view<T, -1>(storage.data(), storage.size());
    }

    constexpr view<T const, -1> as_view() const {
        return view<T const, -1>(storage.data(), storage.size());
    }
};


/*
 * A fixed-shape view.
 */
template <typename T, dim_t Head, dim_t... Tail>
struct view {

    static constexpr unsigned ndim = 1 + sizeof...(Tail);
    static constexpr unsigned nddim = 0;

    T* pointer;

    constexpr explicit view(T* pointer) : pointer(pointer) {}

    constexpr view(T* pointer, shape_t<Head, Tail...> const&) : view(pointer) {}

    constexpr T* data() noexcept {
        return pointer;
    }

    constexpr T const* data() const noexcept {
        return pointer;
    }

    constexpr size_t size() const noexcept {
        return (Head * ... * Tail);
    }

    constexpr shape_t<Head, Tail...> shape() const noexcept {
        return {};
    }

    constexpr view<T, Tail...> operator[](dim_t index) {
        return view<T, Tail...>(pointer + index * (Tail * ...));
    }

    constexpr view<T const, Tail...> operator[](dim_t index) const {
        return view<T const, Tail...>(pointer + index * (Tail * ...));
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        std::fill_n(pointer, size(), value);
    }

    constexpr tensor<T, Head, Tail...> as_tensor() const {
        tensor<T, Head, Tail...> result;
        std::copy_n(pointer, size(), result.data());
        return result;
    }

    constexpr view<T, Head, Tail...> const& as_view() {
        return *this;
    }

    constexpr view<T const, Head, Tail...> as_view() const {
        return view<T const, Head, Tail...>(pointer);
    }
};


/*
 * A dynamic-shaped view, where variable-size dimensions are explicitly stored.
 */
template <typename T, dim_t Head, dim_t... Tail>
    requires ((Head < 0) || ... || (Tail < 0))
struct view<T, Head, Tail...> {

    static constexpr unsigned ndim = 1 + sizeof...(Tail);
    static constexpr unsigned nddim = ((Head < 0) + ... + (Tail < 0));

    T* pointer;
    shape_t<Head, Tail...> shape_;

    constexpr view(T* pointer, shape_t<Head, Tail...> const& shape) : pointer(pointer), shape_(shape) {}

    template <typename... S>
    constexpr explicit view(T* pointer, dim_t h, S... t) : view(pointer, shape_t<Head, Tail...>{ h, dim_t(t)... }) {}

    constexpr T* data() noexcept {
        return pointer;
    }

    constexpr T const* data() const noexcept {
        return pointer;
    }

    constexpr size_t size() const noexcept {
        return shape_.product();
    }

    constexpr shape_t<Head, Tail...> shape() const {
        return shape_;
    }

    constexpr view<T, Tail...> operator[](dim_t index) {
        return { pointer + index * shape_.tail().product(), shape_.tail() };
    }

    constexpr view<T const, Tail...> operator[](dim_t index) const {
        return { pointer + index * shape_.tail().product(), shape_.tail() };
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        std::fill_n(pointer, size(), value);
    }

    constexpr tensor<T, Head, Tail...> as_tensor() const {
        tensor<T, Head, Tail...> result(shape_);
        std::copy_n(pointer, size(), result.data());
        return result;
    }

    constexpr view<T, Head, Tail...> const& as_view() {
        return *this;
    }

    constexpr view<T const, Head, Tail...> as_view() const {
        return view<T const, Head, Tail...>(pointer, shape_);
    }
};


/*
 * One-dimensional specialization, as the accessor must return a reference to a
 * scalar.
 */
template <typename T, dim_t Head>
struct view<T, Head> {

    static constexpr unsigned ndim = 1;
    static constexpr unsigned nddim = 0;

    T* pointer;

    constexpr explicit view(T* pointer) : pointer(pointer) {}

    constexpr view(T* pointer, shape_t<Head> const&) : view(pointer) {}

    constexpr T* data() noexcept {
        return pointer;
    }

    constexpr T const* data() const noexcept {
        return pointer;
    }

    constexpr size_t size() const noexcept {
        return Head;
    }

    constexpr shape_t<Head> shape() const {
        return {};
    }

    constexpr T& operator[](dim_t index) {
        return pointer[index];
    }

    constexpr T const& operator[](dim_t index) const {
        return pointer[index];
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        std::fill_n(pointer, Head, value);
    }

    constexpr tensor<T, Head> as_tensor() const {
        tensor<T, Head> result;
        std::copy_n(pointer, Head, result.data());
        return result;
    }

    constexpr view<T, Head> const& as_view() {
        return *this;
    }

    constexpr view<T const, Head> as_view() const {
        return view<T const, Head>(pointer);
    }
};


/*
 * Finally, the dynamic variant of a one-dimensional view.
 */
template <typename T>
struct view<T, -1> {

    static constexpr unsigned ndim = 1;
    static constexpr unsigned nddim = 1;

    static constexpr std::array<dim_t, ndim> static_shape = { -1 };

    T* pointer;
    shape_t<-1> shape_;

    constexpr view(T* pointer, shape_t<-1> const& shape) : pointer(pointer), shape_(shape) {}

    constexpr explicit view(T* pointer, dim_t h) : view(pointer, shape_t<-1>{ h }) {}

    constexpr T* data() noexcept {
        return pointer;
    }

    constexpr T const* data() const noexcept {
        return pointer;
    }

    constexpr size_t size() const noexcept {
        return shape_.head();
    }

    constexpr shape_t<-1> shape() const noexcept {
        return shape_;
    }

    constexpr T& operator[](dim_t index) {
        return pointer[index];
    }

    constexpr T const& operator[](dim_t index) const {
        return pointer[index];
    }

    // TODO at
    // TODO operator()

    // TODO =

    constexpr void fill(T const& value) {
        std::fill_n(pointer, shape_.head(), value);
    }

    constexpr tensor<T, -1> as_tensor() const {
        tensor<T, -1> result(shape_);
        std::copy_n(pointer, shape_.head(), result.data());
        return result;
    }

    constexpr view<T, -1> const& as_view() {
        return *this;
    }

    constexpr view<T const, -1> as_view() const {
        return view<T const, -1>(pointer, shape_.head());
    }
};


// TODO arithmetic operators?


#define GAME_TENSOR_MAKE_OPERATORS(ltype, rtype)                                           \
                                                                                           \
template <typename T, dim_t... L, dim_t... R>                                              \
constexpr bool operator==(ltype<T, L...> const& left, rtype<T, R...> const& right) {       \
    if (left.shape() != right.shape())                                                     \
        return false;                                                                      \
    return std::equal(left.data(), left.data() + left.shape().product(), right.data());    \
}                                                                                          \
                                                                                           \
template <typename T, dim_t... L, dim_t... R>                                              \
constexpr auto operator<=>(ltype<T, L...> const& left, rtype<T, R...> const& right) {      \
    return std::lexicographical_compare_three_way(                                         \
        left.data(), left.data() + left.size(),                                            \
        right.data(), right.data() + right.size()                                          \
    );                                                                                     \
}                                                                                          \

GAME_TENSOR_MAKE_OPERATORS(tensor, tensor)
GAME_TENSOR_MAKE_OPERATORS(tensor, view)
GAME_TENSOR_MAKE_OPERATORS(view, tensor)
GAME_TENSOR_MAKE_OPERATORS(view, view)

#undef GAME_TENSOR_MAKE_OPERATORS


template <typename T, dim_t Head, dim_t... Tail>
struct hash<tensor<T, Head, Tail...>> {
    constexpr size_t operator()(tensor<T, Head, Tail...> const& value) const {
        return hash_value(value.storage);
    }
};

template <typename T, dim_t Head, dim_t... Tail>
struct hash<view<T, Head, Tail...>> {
    constexpr size_t operator()(view<T, Head, Tail...> const& value) const {
        return hash_range(value.data(), value.data() + value.size());
    }
};


}


#endif
