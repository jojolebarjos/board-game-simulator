#ifndef GAME_HASH_HPP
#define GAME_HASH_HPP


#include <array>
#include <bit>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>


namespace game {


static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);

static constexpr size_t HASH_BASIS = sizeof(size_t) == 4 ? 0x811c9dc5 : 0xcbf29ce484222325;
static constexpr size_t HASH_PRIME = sizeof(size_t) == 4 ? 0x01000193 : 0x100000001b3;


template <typename T>
struct Hash {
	constexpr size_t operator()(T const& value) noexcept;
};


template <typename T>
constexpr size_t hash_of(T const& value) noexcept {
	Hash<T> h;
	return h(value);
}


template <typename... Ts>
constexpr size_t hash(Ts const&... value) noexcept {
	size_t result = HASH_BASIS;
	(((result ^= hash_of(value)) *= HASH_PRIME), ...);
	return result;
}


#define GAME_HASH_SIMPLE(TYPE)                               \
template <>													 \
struct Hash<TYPE> {										     \
	constexpr size_t operator()(TYPE value) noexcept {	     \
		return value;										 \
	}														 \
};															 \

GAME_HASH_SIMPLE(char)
GAME_HASH_SIMPLE(int8_t)
GAME_HASH_SIMPLE(uint8_t)
GAME_HASH_SIMPLE(int16_t)
GAME_HASH_SIMPLE(uint16_t)
GAME_HASH_SIMPLE(int32_t)
GAME_HASH_SIMPLE(uint32_t)
GAME_HASH_SIMPLE(int64_t)
GAME_HASH_SIMPLE(uint64_t)

#undef GAME_HASH_SIMPLE


template <>
struct Hash<float> {
	constexpr size_t operator()(float value) noexcept {
		if (value == 0.0f || value != value)
			value = 0.0f;
		return hash(std::bit_cast<uint32_t>(value));
	}
};


template <>
struct Hash<double> {
	constexpr size_t operator()(double value) noexcept {
		if (value == 0.0 || value != value)
			value = 0.0;
		return hash(std::bit_cast<uint64_t>(value));
	}
};


template <typename... Ts>
struct Hash<std::tuple<Ts...>> {
	constexpr size_t operator()(std::tuple<Ts...> const& value) noexcept {
		return std::apply(hash, value);
	}
};


template <typename T, size_t N>
struct Hash<std::array<T, N>> {
	constexpr size_t operator()(std::array<T, N> const& value) noexcept {
		size_t result = HASH_BASIS;
		for (size_t i = 0; i < N; ++i) {
			result ^= hash_of(value[i]);
			result *= HASH_PRIME;
		}
		return result;
	}
};


template <typename T, typename Allocator>
struct Hash<std::vector<T, Allocator>> {
	constexpr size_t operator()(std::vector<T, Allocator> const& value) noexcept {
		size_t result = HASH_BASIS;
		for (size_t i = 0; i < value.size(); ++i) {
			result ^= hash_of(value[i]);
			result *= HASH_PRIME;
		}
		return result;
	}
};


}


#endif
