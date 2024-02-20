#ifndef GAME_CONNECT_HPP
#define GAME_CONNECT_HPP


#include <algorithm>
#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

#include "./hash.hpp"
#include "./tensor.hpp"


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
    tensor<int8_t, H, W> grid;
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

    constexpr void validate() const {
        for (unsigned j = 0; j < W; ++j) {
            for (unsigned i = 0; i < H; ++i) {
                if (grid[i][j] < -1 || grid[i][j] > 1)
                    throw std::runtime_error("grid contains invalid player indices");
            }
            // TODO check that grid is valid (e.g. no floating value)
        }
        // TODO check player and winner
    }

    constexpr void swap() {
        for (unsigned i = 0; i < H; ++i) {
            for (unsigned j = 0; j < W; ++j) {
                if (grid[i][j] >= 0)
                    grid[i][j] = 1 - grid[i][j];
            }
        }
        if (player >= 0)
            player = 1 - player;
        else if (winner >= 0)
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
        for (unsigned i_ = i, j_ = j; i_ > 0 && j_ < W - 1 && grid[--i_][++j_] == who; ++b);
        for (unsigned i_ = i, j_ = j; i_ < H - 1 && j_ > 0 && grid[++i_][--j_] == who; ++b);

        return std::max({ h, v, a, b });
    }

    constexpr bool is_full() noexcept {
        for (unsigned j = 0; j < W; ++j)
            if (grid[H - 1][j] < 0)
                return false;
        return true;
    }

    constexpr void apply(Action action) {
        unsigned j = action;
        for (unsigned i = 0; i < H; ++i) {
            if (grid[i][j] < 0) {
                grid[i][j] = player;

                // Check whether this is a win
                if (count_at(i, j) >= N) {
                    winner = player;
                    player = -1;
                    return;
                }

                // Check whether this is a draw
                if (is_full()) {
                    player = -1;
                    return;
                }

                break;
            }
        }

        // Game has not ended
        player = player ? 0 : 1;
    }

    constexpr void get_actions(std::vector<Action>& actions) const {
        if (player >= 0) {
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
    using State = game::connect::State<H, W, N>;
    using Action = game::connect::Action;

    // TODO should player count be static constexpr?

    static constexpr void initialize(State& state) {
        state.initialize();
    }

    static constexpr bool has_ended(State const& state) noexcept {
        return state.player < 0;
    }

    static constexpr int get_player(State const& state) noexcept {
        return state.player;
    }

    static constexpr int get_winner(State const& state) noexcept {
        return state.winner;
    }

    static constexpr tensor<float, 2> get_reward(State const& state) noexcept {
        // TODO what about intermediate rewards?
        switch (state.winner) {
        case 0:
            return { 1.0f, -1.0f };
        case 1:
            return { -1.0f, 1.0f };
        default:
            return { 0.0f, 0.0f };
        }
    }

    static constexpr std::tuple<tensor<int8_t, H, W>> get_tensors(State const& state) {
        // TODO should maybe accept some "option" argument?
        // TODO actual implementation
        return { state.grid };
    }

    // TODO add data augmentation helper (i.e. mirror)

    static constexpr void get_actions(State const& state, std::vector<Action>& actions) {
        state.get_actions(actions);
    }

    static constexpr void apply(State& state, Action action) {
        state.apply(action);
    }

    static json to_json(State const& state) {
        // TODO maybe player/winner should be null?
        return {
            {"grid", state.grid},
            {"player", state.player},
            {"winner", state.winner}
        };
    }

    static json to_json(State const& state, Action action) {
        return action;
    }

    static constexpr void from_json(State& state, json const& j) {
        j.at("grid").get_to(state.grid);
        j.at("player").get_to(state.player);
        j.at("winner").get_to(state.winner);
        state.validate();
    }

    static constexpr void from_json(State const& state, Action& action, json const& j) {
        j.get_to(action);
        if (j >= W)
            throw std::runtime_error("column index is out-of-bounds");
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
        return Hash::compute(state.grid, state.player);
    }

    static constexpr size_t hash(State const& state, Action action) noexcept {
        return Hash::compute(state.grid, state.player, action);
    }
};


}
}


#endif
