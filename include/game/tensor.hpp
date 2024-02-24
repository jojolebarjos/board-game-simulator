#ifndef GAME_TENSOR_HPP
#define GAME_TENSOR_HPP


#include <array>

#include <nlohmann/json.hpp>


namespace game {


template <typename T, size_t... Shape>
struct tensor;


namespace detail {

template <typename T, size_t Head, size_t... Tail>
struct tensor_types {
	using value_type = tensor<T, Tail...>;
	using data_type = std::array<value_type, Head>;
};


template <typename T, size_t Head>
struct tensor_types<T, Head> {
	using value_type = T;
	using data_type = std::array<value_type, Head>;
};

}


template <typename T, size_t... Shape>
struct tensor {

	using value_type = typename detail::tensor_types<T, Shape...>::value_type;
	using data_type = typename detail::tensor_types<T, Shape...>::data_type;

	data_type values;

	static constexpr size_t ndim = sizeof...(Shape);
	static constexpr std::array<size_t, ndim> shape = { Shape... };

	template <typename U>
	explicit constexpr operator tensor<U, Shape...>() const noexcept {
		tensor<U, Shape...> result;
		for (size_t i = 0; i < values.size(); ++i)
			result[i] = static_cast<tensor<U, Shape...>::value_type>(values[i]);
		return result;
	}

	constexpr size_t size() const noexcept {
		return values.size();
	}

	constexpr value_type* data() noexcept {
		return values.data();
	}

	constexpr value_type const* data() const noexcept {
		return values.data();
	}

	constexpr auto const& operator[](size_t i) const noexcept {
		return values[i];
	}

	constexpr auto& operator[](size_t i) noexcept {
		return values[i];
	}

	constexpr auto const& at(size_t i) const {
		return values.at(i);
	}

	constexpr auto& at(size_t i) {
		return values.at(i);
	}

	constexpr auto begin() const noexcept {
		return values.begin();
	}

	constexpr auto begin() noexcept {
		return values.begin();
	}

	constexpr auto end() const noexcept {
		return values.end();
	}

	constexpr auto end() noexcept {
		return values.end();
	}

	constexpr auto operator<=>(tensor<T, Shape...> const& right) const noexcept = default;
};


template <typename T, size_t... Shape>
constexpr auto operator+(tensor<T, Shape...> const& right) noexcept {
	return right;
}


template <typename T, size_t... Shape>
constexpr auto operator-(tensor<T, Shape...> const& right) noexcept {
	tensor<T, Shape...> result;
	for (size_t i = 0; i < right.size(); ++i)
		result[i] = -right[i];
	return result;
}


#define GAME_TENSOR_OP(OP)                                                                                 \
                                                                                                           \
template <typename T, size_t... Shape>                                                                     \
constexpr auto& operator OP##=(tensor<T, Shape...>& left, tensor<T, Shape...> const& right) noexcept {     \
	for (size_t i = 0; i < left.size(); ++i)                                                               \
		left[i] OP##= right[i];                                                                            \
	return left;                                                                                           \
}                                                                                                          \
                                                                                                           \
template <typename T, size_t... Shape>                                                                     \
constexpr auto operator OP(tensor<T, Shape...> const& left, tensor<T, Shape...> const& right) noexcept {   \
	tensor<T, Shape...> result(left);                                                                      \
	return result OP##= right;                                                                             \
}                                                                                                          \
                                                                                                           \
template <typename T, size_t... Shape>                                                                     \
constexpr auto& operator OP##=(tensor<T, Shape...>& left, T const& right) noexcept {                       \
	for (size_t i = 0; i < left.size(); ++i)                                                               \
		left[i] OP##= right;                                                                               \
	return left;                                                                                           \
}                                                                                                          \
                                                                                                           \
template <typename T, size_t... Shape>                                                                     \
constexpr auto operator OP(tensor<T, Shape...> const& left, T const& right) noexcept {                     \
	tensor<T, Shape...> result(left);                                                                      \
	return result OP##= right;                                                                             \
}                                                                                                          \
                                                                                                           \
template <typename T, size_t... Shape>                                                                     \
constexpr auto operator OP(T const& left, tensor<T, Shape...> const& right) noexcept {                     \
	tensor<T, Shape...> result;                                                                            \
	for (size_t i = 0; i < right.size(); ++i)                                                              \
		result[i] = left OP right[i];                                                                      \
	return result;                                                                                         \
}                                                                                                          \

GAME_TENSOR_OP(+)
GAME_TENSOR_OP(-)
GAME_TENSOR_OP(*)
GAME_TENSOR_OP(/)
GAME_TENSOR_OP(%)

#undef GAME_TENSOR_OP


template <typename T, size_t... Shape>
void to_json(nlohmann::json& j, tensor<T, Shape...> const& value) {
	j = nlohmann::json(value.values);
}


template <typename T, size_t... Shape>
void from_json(nlohmann::json const& j, tensor<T, Shape...>& value) {
	j.get_to(value.values);
}


}


#endif
