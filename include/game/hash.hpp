#ifndef GAME_HASH_HPP
#define GAME_HASH_HPP


#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>


namespace game {


/*
 * Both in C++ and CPython, the hash seed is randomized per-process (unless explicitly
 * disabled):
 *   https://en.cppreference.com/w/cpp/utility/hash
 *   https://docs.python.org/3/using/cmdline.html#envvar-PYTHONHASHSEED
 * Therefore, in general, hashes should not be assumed to be re-usable across
 * executions. Note that in CPython, not all types are affected by this randomization.
 *
 * Boost, on the other hand, is completely deterministic. Let us base our
 * implementation on it.
 *   https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html
 *
 * Also note that there was a proposal for C++ that suggested similar extensions:
 *   https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3333.html
 */


static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);


/*
 * We will rely on FNV-1a:
 *   https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 * A lot could be debated about computational efficiency, and dispersion properties.
 */

// TODO should probably use https://en.wikipedia.org/wiki/SipHash
static constexpr size_t hash_basis = sizeof(size_t) == 4 ? 0x811c9dc5 : 0xcbf29ce484222325;
static constexpr size_t hash_prime = sizeof(size_t) == 4 ? 0x01000193 : 0x100000001b3;


/*
 * Main hash definition, new types can be supported by template specialization.
 *
 * In order to do some weird tricks, the Enable template argument is added:
 *   https://quuxplusone.github.io/blog/2019/04/26/std-hash-should-take-another-template-parameter/
 */

template <typename T, typename Enable = void>
struct hash {
    static_assert(sizeof(T) == 0);

    size_t operator()(T const&) const {
        return 0;
    }
};


/*
 * In std::hash, the main approach to extend hashing to custom types is by template
 * specialization. In Boost, the boost::hash_value must be overloaded. Here, we follow
 * the approach in the standard library; a hash_value function is provided for
 * convenience.
 */

template <typename T>
size_t hash_value(T const& value) {
    hash<T> hasher;
    return hasher(value);
}


/*
 * A few helpers to hash multiple values.
 */

template <typename T>
void hash_combine(size_t& seed, T const& value) {
    seed ^= hash_value(value);
    seed *= hash_prime;
}

template <typename... Ts>
size_t hash_many(Ts const&... values) {
    size_t seed = hash_basis;
    (hash_combine(seed, values), ...);
    return seed;
}

template <typename It>
size_t hash_range(It first, It last) {
    size_t seed = hash_basis;
    for (; first != last; ++first)
        hash_combine(seed, *first);
    return seed;
}

template <typename Container>
size_t hash_range(Container const& container) {
    return hash_range(container.begin(), container.end());
}


/*
 * Simple integral types will just be their own hash, as done in most implementations.
 */

#define GAME_HASH_SIMPLE(TYPE)                                \
template <>                                                   \
struct hash<TYPE> {                                           \
    constexpr size_t operator()(TYPE const& value) const {    \
        return (size_t)value;                                 \
    }                                                         \
};                                                            \

GAME_HASH_SIMPLE(char)
GAME_HASH_SIMPLE(unsigned char)
GAME_HASH_SIMPLE(signed char)
GAME_HASH_SIMPLE(unsigned short)
GAME_HASH_SIMPLE(signed short)
GAME_HASH_SIMPLE(unsigned int)
GAME_HASH_SIMPLE(signed int)
GAME_HASH_SIMPLE(unsigned long)
GAME_HASH_SIMPLE(signed long)
GAME_HASH_SIMPLE(unsigned long long)
GAME_HASH_SIMPLE(signed long long)

#undef GAME_HASH_SIMPLE


/*
 * Specializations for floating-point numbers, since some special values may have
 * multiple binary representations.
 */

template <>
struct hash<float> {
    size_t operator()(float value) const {
        if (value == 0.0f || value != value)
            value = 0.0f;
        return hash_value(std::bit_cast<uint32_t>(value));
    }
};

template <>
struct hash<double> {
    size_t operator()(double value) const {
        if (value == 0.0 || value != value)
            value = 0.0;
        return hash_value(std::bit_cast<uint64_t>(value));
    }
};


/*
 * Specializations for some STL containers.
 */

template <typename... T>
struct hash<std::tuple<T...>> {
    size_t operator()(std::tuple<T...> const& value) const {
        return std::apply(hash_many<T...>, value);
    }
};

template <typename T, size_t N>
struct hash<std::array<T, N>> {
    size_t operator()(std::array<T, N> const& value) const {
        return hash_range(value);
    }
};

template <typename T, typename Allocator>
struct hash<std::vector<T, Allocator>> {
    size_t operator()(std::vector<T, Allocator> const& value) const {
        return hash_range(value);
    }
};


/*
 * For convenience, a pointer is hashed based on its value, not the address in memory.
 * The latter is almost never what we would like to do...
 */

template <typename T>
struct hash<std::shared_ptr<T>> {
    size_t operator()(std::shared_ptr<T> const& value) const {
        return hash_value(*value);
    }
};



}


#endif
