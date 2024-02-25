#ifndef GAME_CHINESE_CHECKERS_HPP
#define GAME_CHINESE_CHECKERS_HPP


#include <algorithm>
#include <cstdint>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

#include "./hash.hpp"
#include "./tensor.hpp"


namespace game {
namespace chinese_checkers {


/*
  We always assume player 0 is the one playing. They have to go to the top-right corner.
  Also note that, by design, a player has ALWAYS at least one move (i.e. the opponent has not enough pieces to block completely).
  There are 121 tiles in total.

          .
          |\
          . .
          |  \
          . . .
          |    \
          . . . .
          |      \
  ._._._._. . . . ._1_1_1_1
   \                      |
    . . . . . . . . . 1 1 1
     \                    |
      . . . . . . . . . 1 1
       \                  |
        . . . . . . . . . 1
         \                |
          . . . . . . . . .
          |                \
          0 . . . . . . . . .
          |                  \
          0 0 . . . . . . . . .
          |                    \
          0 0 0 . . . . . . . . .
          |                      \
          0_0_0_0_. . . . ._._._._.
                   \      |
                    . . . .
                     \    |
                      . . .
  y                    \  |
  ^                     . .
  |                      \|
  o-> x                   .

  https://en.wikipedia.org/wiki/Chinese_checkers
  https://www.ymimports.com/pages/how-to-play-chinesse-checkers
*/


constexpr unsigned NUM_PIECES_PER_PLAYER = 10;
constexpr unsigned NUM_PLAYERS = 2;
constexpr unsigned GRID_SIZE = 17;


typedef tensor<int8_t, 2> Coordinate;


constexpr Coordinate UPPER_RIGHT = { GRID_SIZE - 1 , GRID_SIZE - 1 };


struct Action {
    uint8_t index;
    Coordinate destination;

    constexpr void swap() noexcept {
        index ^= 1;
        destination = UPPER_RIGHT - destination;
    }

    constexpr auto operator<=>(Action const& other) const noexcept = default;
};


struct State {
    tensor<Coordinate, NUM_PLAYERS, NUM_PIECES_PER_PLAYER> pieces;
    int8_t player;
    int8_t winner;

    static constexpr tensor<Coordinate, NUM_PLAYERS, NUM_PIECES_PER_PLAYER> BASE_COORDINATES = {
         4,  4,
         4,  5,
         4,  6,
         4,  7,
         5,  4,
         5,  5,
         5,  6,
         6,  4,
         6,  5,
         7,  4,
         9, 12,
        10, 11,
        10, 12,
        11, 10,
        11, 11,
        11, 12,
        12,  9,
        12, 10,
        12, 11,
        12, 12,
    };

    constexpr void initialize() noexcept {
        pieces = BASE_COORDINATES;
        player = 0;
        winner = -1;
    }

    constexpr void validate() const {
        // TODO
    }

    constexpr void mirror() noexcept {
        for (unsigned i = 0; i < NUM_PLAYERS; ++i) {
            for (unsigned j = 0; j < NUM_PIECES_PER_PLAYER; ++j) {
                Coordinate& c = pieces[i][j];
                std::swap(c[0], c[1]);
            }
        }
    }

    constexpr void swap() noexcept {
        player ^= 1;
        if (winner >= 0) {
            winner ^= 1;
        }
        for (unsigned j = 0; j < NUM_PIECES_PER_PLAYER; ++j) {
            Coordinate a = pieces[0][j];
            Coordinate b = pieces[1][j];
            pieces[0][j] = UPPER_RIGHT - b;
            pieces[1][j] = UPPER_RIGHT - a;
        }
    }

    constexpr void apply(Action const& action) noexcept {
        pieces[player][action.index] = action.destination;
        std::sort(pieces[player].begin(), pieces[player].end());
        if (std::equal(pieces[player].begin(), pieces[player].end(), BASE_COORDINATES[player ^ 1].begin())) {
            winner = player;
        }
        player ^= 1;
    }

    constexpr auto operator<=>(State const& other) const noexcept = default;
};


constexpr tensor<int8_t, GRID_SIZE, GRID_SIZE> EMPTY_GRID = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9,
    9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9,
    9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9,
    9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9,
    9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9,
    9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
};


struct Grid {
    State const* state;
    tensor<int8_t, GRID_SIZE, GRID_SIZE> grid;

    constexpr void initialize(State const& state) noexcept {
        this->state = &state;
        grid = EMPTY_GRID;
        for (unsigned i = 0; i < NUM_PLAYERS; ++i) {
            for (unsigned j = 0; j < NUM_PIECES_PER_PLAYER; ++j) {
                Coordinate c = state.pieces[i][j];
                grid[c[1]][c[0]] = i + 1;
            }
        }
    }

    constexpr bool is_free(Coordinate c) const noexcept {
        if (c[0] < 0 || c[1] < 0 || c[0] >= GRID_SIZE || c[1] >= GRID_SIZE)
            return false;
        return !grid[c[1]][c[0]];
    }

    constexpr void generate_actions_single(std::vector<Action>& actions, unsigned index) noexcept {
        auto start = actions.size();

        constexpr Coordinate deltas[] = {
            { 1, 0 },
            { 0, 1 },
            { -1, 1 },
            { -1, 0 },
            { 0, -1 },
            { 1, -1 },
        };

        Coordinate origin = state->pieces[state->player][index];

        // Enumerate hops, using Depth First Search
        // Note: marking locations as visited directly in grid
        constexpr unsigned MAX_DEPTH = 60;
        Coordinate stack[MAX_DEPTH];
        stack[0] = origin;
        unsigned depth = 1;
        while (depth > 0) {
            Coordinate location = stack[--depth];
            for (Coordinate delta : deltas) {
                Coordinate anchor = location + delta;
                if (!is_free(anchor)) {
                    Coordinate hop = anchor + delta;
                    if (is_free(hop)) {
                        grid[hop[1]][hop[0]] = -1;
                        stack[depth++] = hop;
                        actions.emplace_back((uint8_t)index, hop);
                    }
                }
            }
        }

        // Clear visited locations
        for (auto i = start; i < actions.size(); ++i) {
            Coordinate location = actions[i].destination;
            grid[location[1]][location[0]] = 0;
        }

        // Enumerate normal moves
        for (Coordinate delta : deltas) {
            Coordinate destination = origin + delta;
            if (is_free(destination)) {
                actions.emplace_back((uint8_t)index, destination);
            }
        }
    }

    constexpr void generate_actions(std::vector<Action>& actions) noexcept {
        for (unsigned j = 0; j < NUM_PIECES_PER_PLAYER; ++j) {
            generate_actions_single(actions, j);
        }
    }
};


struct Traits {
    using State = game::chinese_checkers::State;
    using Action = game::chinese_checkers::Action;

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

    static constexpr std::tuple<> get_tensors(State const& state) {
        // TODO actual implementation
        return {};
    }

    // TODO add data augmentation helper (i.e. mirror)

    static constexpr void get_actions(State const& state, std::vector<Action>& actions) {
        Grid grid;
        grid.initialize(state);
        grid.generate_actions(actions);
    }

    static constexpr void apply(State& state, Action const& action) {
        state.apply(action);
    }

    static nlohmann::json to_json(State const& state) {
        return {
            {"pieces", state.pieces},
            {"player", state.player},
            {"winner", state.winner}
        };
    }

    static nlohmann::json to_json(State const& state, Action const& action) {
        return {
            {"index", action.index},
            {"x", action.destination[0]},
            {"y", action.destination[1]}
        };
    }

    static void from_json(State& state, nlohmann::json const& j) {
        j.at("pieces").get_to(state.pieces);
        j.at("player").get_to(state.player);
        j.at("winner").get_to(state.winner);
        state.validate();
    }

    static void from_json(State const& state, Action& action, nlohmann::json const& j) {
        j.at("index").get_to(action.index);
        j.at("x").get_to(action.destination[0]);
        j.at("y").get_to(action.destination[1]);
        // TODO validate that action is valid
    }

    static constexpr auto compare(State const& left, State const& right) noexcept {
        return left <=> right;
    }

    static constexpr auto compare(State const& left, Action const& left_action, State const& right, Action const& right_action) noexcept {
        if (auto cmp = left <=> right; cmp != 0)
            return cmp;
        return left_action <=> right_action;
    }

    static constexpr size_t hash(State const& state) noexcept {
        return game::hash(state.pieces, state.player);
    }

    static constexpr size_t hash(State const& state, Action const& action) noexcept {
        return game::hash(state.pieces, state.player, action.index, action.destination);
    }
};


}
}


#endif
