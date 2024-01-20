#ifndef GAME_CONNECT_HPP
#define GAME_CONNECT_HPP


#include <algorithm>
#include <cstdint>
#include <vector>


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
struct Context {
    using State = State<H, W, N>;

    constexpr void clear() {}

    constexpr State sample_initial_state() {
        State state;
        state.initialize();
        return state;
    }

    constexpr unsigned get_player(State const& state) {
        return state.player;
    }

    constexpr bool has_ended(State const& state) {
        return state.winner >= 0;
    }

    // Reward get_reward(State const& state) {

    constexpr void get_actions(std::vector<Action>& actions, State const& state) {
        state.get_actions(actions);
    }

    constexpr State sample_next_state(State const& state, Action action) {
        State next_state = state;
        next_state.apply(action);
        return next_state;
    }

    // float get_reward_for(Reward reward, State const& state) {
};


template <unsigned H, unsigned W, unsigned N>
struct Traits {
    typedef State<H, W, N> State;
    typedef Action Action;
    typedef Context<H, W, N> Context;
};


}
}


#endif
