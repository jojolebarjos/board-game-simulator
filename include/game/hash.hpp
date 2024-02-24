#ifndef GAME_HASH_HPP
#define GAME_HASH_HPP


#include <cstdint>
#include <type_traits>


namespace game {


struct Hash {

	static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);

	static constexpr size_t HASH_BASIS = sizeof(size_t) == 4 ? 0x811c9dc5 : 0xcbf29ce484222325;
	static constexpr size_t HASH_PRIME = sizeof(size_t) == 4 ? 0x01000193 : 0x100000001b3;

	size_t value;

	constexpr Hash(size_t value = HASH_BASIS) noexcept : value(value) {}

	constexpr void update(void const* data, size_t size) noexcept {
		for (size_t i = 0; i < size; ++i) {
			value ^= ((uint8_t const*)data)[i];
			value *= HASH_PRIME;
		}
	}

	constexpr void update() noexcept {}

	// TODO remove this, and use proper type-safe hash (e.g. careful with floats...)
	template <typename T>
	constexpr void update(T const& data) noexcept {
		update(reinterpret_cast<void const*>(&data), sizeof(data));
	}

	template <typename Head, typename... Tail>
	constexpr void update(Head const& head, Tail const &... tail) noexcept {
		update(head);
		update(tail...);
	}

	template <typename... Ts>
	static constexpr size_t compute(Ts const&... values) noexcept {
		Hash h;
		h.update(values...);
		return h.value;
	}
};


}


#endif
