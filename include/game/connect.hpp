#ifndef GAME_CONNECT_HPP
#define GAME_CONNECT_HPP


#include <algorithm>
#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>


namespace game {
namespace connect {


/*
  The origin of the grid is the lower-left corner. A typical Connect4 board has
  size 6x7:

  . . . . . . .
  . . . . . . .
  . . . . . . .
  . . . . . . .
  . . O O . . .
  . X O X . . X
*/


using json = nlohmann::json;


typedef uint8_t Action;


// TODO should we allow more than 2 players?

template <unsigned H, unsigned W, unsigned N>
struct State {
    int8_t grid[H][W];
    int8_t player;
    int8_t winner;

    constexpr void initialize() {
        for (unsigned i = 0; i < H; ++i) {
            for (unsigned j = 0; j < W; ++j) {
                grid[i][j] = -1;
            }
        }
        player = 0;
        winner = -1;
    }

    constexpr void swap() {
        for (unsigned i = 0; i < H; ++i) {
            for (unsigned j = 0; j < W; ++j) {
                if (grid[i][j] >= 0)
                    grid[i][j] = 1 - grid[i][j];
            }
        }
        player = 1 - player;
        if (winner >= 0)
            winner = 1 - winner;
    }

    constexpr void mirror() {
        for (unsigned i = 0; i < H; ++i) {
            for (unsigned j = 0; j < W / 2; ++j) {
                std::swap(grid[i][j], grid[i][W - j - 1]);
            }
        }
    }

    constexpr unsigned count_at(unsigned i, unsigned j) const {
        unsigned who = grid[i][j];

        unsigned h = 1;
        for (unsigned j_ = j; j_ > 0 && grid[i][--j_] == who; ++h);
        for (unsigned j_ = j; j_ < W - 1 && grid[i][++j_] == who; ++h);

        unsigned v = 1;
        for (unsigned i_ = i; i_ > 0 && grid[--i_][j] == who; ++v);
        for (unsigned i_ = i; i_ < H - 1 && grid[++i_][j] == who; ++v);

        unsigned a = 1;
        for (unsigned i_ = i, j_ = j; i_ > 0 && j_ > 0 && grid[--i_][--j_] == who; ++a);
        for (unsigned i_ = i, j_ = j; i_ < H - 1 && j_ < W - 1 && grid[++i_][++j_] == who; ++a);

        unsigned b = 1;
        for (unsigned i_ = i, j_ = j; i_ > 0 && j_ < W - 1 && grid[--i_][++j_] == who; ++a);
        for (unsigned i_ = i, j_ = j; i_ < H - 1 && j_ > 0 && grid[++i_][--j_] == who; ++a);

        return std::max({ h, v, a, b });
    }

    constexpr void apply(Action action) {
        unsigned j = action;
        for (unsigned i = 0; i < H; ++i) {
            if (grid[i][j] < 0) {
                grid[i][j] = player;
                if (count_at(i, j) >= N) {
                    winner = player;
                }
                break;
            }
        }
        player = player ? 0 : 1;
    }

    constexpr void get_actions(std::vector<Action>& actions) const {
        if (winner < 0) {
            for (unsigned j = 0; j < W; ++j) {
                if (grid[H - 1][j] < 0) {
                    actions.push_back(j);
                }
            }
        }
    }

    constexpr auto operator<=>(State const& other) const = default;
};


template <unsigned H, unsigned W, unsigned N>
struct Traits {
    // TODO reward object?
    typedef State<H, W, N> State;
    typedef Action Action;

    static constexpr void initialize(State& state) {
        state.initialize();
    }

    static constexpr bool has_ended(State const& state) noexcept {
        return state.player == -1;
    }

    static constexpr int get_player(State const& state) noexcept {
        return state.player;
    }

    static constexpr int get_winner(State const& state) noexcept {
        return state.winner;
    }

    // TODO reward

    // TODO tensor representation

    static constexpr void get_actions(State const& state, std::vector<Action>& actions) {
        state.get_actions(actions);
    }

    static constexpr void apply(State& state, Action action) {
        state.apply(action);
    }

    static constexpr json to_json(State const& state) {
        // TODO
        json j =
        {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Niels"},
            {"nothing", nullptr},
            {
                "answer", {
                    {"everything", 42}
                }
            },
            {"list", {1, 0, 2}},
            {
                "object", {
                    {"currency", "USD"},
                    {"value", 42.99}
                }
            }
        };
        return j;
    }

    static constexpr json to_json(State const& state, Action action) {
        return action;
    }

    static constexpr void from_json(State& state, json const& j) {
        // TODO
        throw std::runtime_error("not implemented");
    }

    static constexpr void from_json(State const& state, Action& action, json const& j) {
        // TODO
        throw std::runtime_error("not implemented");
    }

    static constexpr auto compare(State const& left, State const& right) noexcept {
        return left <=> right;
    }

    static constexpr auto compare(State const& left, Action left_action, State const& right, Action right_action) noexcept {
        if (auto cmp = left <=> right; cmp != 0)
            return cmp;
        return left_action <=> right_action;
    }

    static constexpr size_t hash(State const& state) noexcept {
        return 42;
    }

    static constexpr size_t hash(State const& state, Action action) noexcept {
        return -1;
    }
};


}
}


#endif
