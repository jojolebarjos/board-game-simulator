#ifndef GAME_TENSOR_HPP
#define GAME_TENSOR_HPP


#include <array>

#include <nlohmann/json.hpp>


namespace game {


using json = nlohmann::json;


// TODO properly add noexcept


template <typename T, size_t Head, size_t... Tail>
struct tensor {
	std::array<tensor<T, Tail...>, Head> data;

	static constexpr size_t dim = 1 + sizeof...(Tail);

	constexpr tensor<T, Tail...> const& operator[](size_t i) const {
		return data[i];
	}

	constexpr tensor<T, Tail...>& operator[](size_t i) {
		return data[i];
	}

	// TODO add operator= for scalar? (would imply explicitly defaulting other assignments)

	constexpr auto operator<=>(tensor<T, Head, Tail...> const& right) const = default;
};


template <typename T, size_t Head>
struct tensor<T, Head> {
	std::array<T, Head> data;

	static constexpr size_t dim = 1;

	constexpr T const& operator[](size_t i) const {
		return data[i];
	}

	constexpr T& operator[](size_t i) {
		return data[i];
	}

	constexpr auto operator<=>(tensor<T, Head> const& right) const = default;
};


template <typename T, size_t Head, size_t... Tail>
constexpr tensor<T, Head, Tail...> operator+(tensor<T, Head, Tail...> const& right) {
	return right;
}


template <typename T, size_t Head, size_t... Tail>
constexpr tensor<T, Head, Tail...> operator-(tensor<T, Head, Tail...> const& right) {
	tensor<T, Head, Tail...> result;
	for (size_t i = 0; i < Head; ++i)
		result[i] = -right[i];
	return result;
}


template <typename T, typename U, size_t Head, size_t... Tail>
constexpr tensor<T, Head, Tail...>& operator+=(tensor<T, Head, Tail...>& left, tensor<U, Head, Tail...> const& right) {
	for (size_t i = 0; i < Head; ++i)
		left[i] += right[i];
	return left;
}


template <typename T, typename U, size_t Head, size_t... Tail>
constexpr tensor<T, Head, Tail...> operator+(tensor<T, Head, Tail...> const& left, tensor<U, Head, Tail...> const& right) {
	tensor<T, Head, Tail...> result(left);
	return result += right;
}


template <typename T, typename U, size_t Head, size_t... Tail>
constexpr tensor<T, Head, Tail...>& operator-=(tensor<T, Head, Tail...>& left, tensor<U, Head, Tail...> const& right) {
	for (size_t i = 0; i < Head; ++i)
		left[i] -= right[i];
	return left;
}


template <typename T, typename U, size_t Head, size_t... Tail>
constexpr tensor<T, Head, Tail...> operator-(tensor<T, Head, Tail...> const& left, tensor<U, Head, Tail...> const& right) {
	tensor<T, Head, Tail...> result(left);
	return result -= right;
}


// TODO operation with scalar
// TODO operation with tensor of smaller dim
// TODO operators *, /, %
// TODO does it make sense to add boolean operators (i.e. ~, &, |, ^)?


template <typename T, size_t Head, size_t... Tail>
void to_json(json& j, tensor<T, Head, Tail...> const& value) {
	j = json(value.data);
}


template <typename T, size_t Head, size_t... Tail>
void from_json(json const& j, tensor<T, Head, Tail...>& value) {
	j.get_to(value.data);
}


}


#endif
