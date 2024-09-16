#ifndef GAME_COMPARISON_HPP
#define GAME_COMPARISON_HPP


#include <tuple>
#include <type_traits>

#include "./hash.hpp"


namespace game {


template <typename Derived>
struct Comparable {

    constexpr bool operator==(Comparable<Derived> const& right) const {
        return derived().get_identity_tuple() == right.derived().get_identity_tuple();
    }

    constexpr auto operator<=>(Comparable<Derived> const& right) const {
        return derived().get_identity_tuple() <=> right.derived().get_identity_tuple();
    }

private:

    friend hash<Derived>;

    constexpr Derived const& derived() const {
        return *static_cast<Derived const*>(this);
    }
};


template <typename Derived>
struct hash<Derived, std::enable_if_t<std::is_base_of_v<Comparable<Derived>, Derived>>> {
    constexpr size_t operator()(Derived const& value) const {
        return hash_value(value.derived().get_identity_tuple());
    }
};


}


#endif
