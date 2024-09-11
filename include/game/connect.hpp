#ifndef GAME_CONNECT_HPP
#define GAME_CONNECT_HPP


#include <memory>
#include <stdexcept>
#include <vector>

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


struct State;
struct Action;


struct Config : std::enable_shared_from_this<Config> {
    // TODO maybe accept more than 2 players?
    static constexpr int num_players = 2;
    int height;
    int width;
    int count;

    Config(int height, int width, int count = 4) :
        height(height),
        width(width),
        count(count)
    {
        if (height < 1 || width < 1 || count < 2)
            throw std::runtime_error("invalid arguments");
    }

    std::shared_ptr<State> sample_initial_state();
};


struct State : std::enable_shared_from_this<State> {
    std::shared_ptr<Config> config;
    tensor<int8_t, -1, -1> grid;
    int8_t player;
    int8_t winner;
    // TODO could store actions as weak_ptr?

    State(std::shared_ptr<Config> const& config) :
        config(config),
        grid(config->height, config->width),
        player(0),
        winner(-1)
    {
        grid.fill(-1);
    }

    bool has_ended() const {
        return player < 0;
    }

    int get_player() const {
        return player;
    }

    tensor<float, 2> get_reward() const {
        switch (winner) {
        case 0:
            return { 1.0f, -1.0f };
        case 1:
            return { -1.0f, 1.0f };
        default:
            return { 0.0f, 0.0f };
        }
    }

    bool is_full() const {
        unsigned height = grid.shape()[0];
        unsigned width = grid.shape()[1];
        for (unsigned j = 0; j < width; ++j)
            if (grid[height - 1][j] < 0)
                return false;
        return true;
    }

    unsigned count_at(unsigned i, unsigned j) const {
        unsigned who = grid[i][j];
        unsigned height = grid.shape()[0];
        unsigned width = grid.shape()[1];

        unsigned h = 1;
        for (unsigned j_ = j; j_ > 0 && grid[i][--j_] == who; ++h);
        for (unsigned j_ = j; j_ < width - 1 && grid[i][++j_] == who; ++h);

        unsigned v = 1;
        for (unsigned i_ = i; i_ > 0 && grid[--i_][j] == who; ++v);
        for (unsigned i_ = i; i_ < height - 1 && grid[++i_][j] == who; ++v);

        unsigned a = 1;
        for (unsigned i_ = i, j_ = j; i_ > 0 && j_ > 0 && grid[--i_][--j_] == who; ++a);
        for (unsigned i_ = i, j_ = j; i_ < height - 1 && j_ < width - 1 && grid[++i_][++j_] == who; ++a);

        unsigned b = 1;
        for (unsigned i_ = i, j_ = j; i_ > 0 && j_ < width - 1 && grid[--i_][++j_] == who; ++b);
        for (unsigned i_ = i, j_ = j; i_ < height - 1 && j_ > 0 && grid[++i_][--j_] == who; ++b);

        return std::max({ h, v, a, b });
    }

    bool can_play_at(unsigned j) const {
        unsigned height = grid.shape()[0];
        unsigned width = grid.shape()[1];
        return !has_ended() && j >= 0 && j < width && grid[height - 1][j] < 0;
    }

    void play_at(unsigned j) {
        if (!can_play_at(j))
            throw std::runtime_error("invalid move");
        unsigned height = grid.shape()[0];
        unsigned count = config->count;
        for (unsigned i = 0; i < height; ++i) {
            if (grid[i][j] < 0) {
                grid[i][j] = player;

                // Check whether this is a win
                if (count_at(i, j) >= count) {
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

    std::shared_ptr<Action> action_at(unsigned j) {
        if (!can_play_at(j))
            throw std::runtime_error("invalid move");
        return std::make_shared<Action>(shared_from_this(), j);
    }

    std::vector<std::shared_ptr<Action>> actions() {
        std::vector<std::shared_ptr<Action>> result;
        for (unsigned j = 0; j < config->width; ++j)
            if (can_play_at(j))
                result.push_back(action_at(j));
        return result;
    }
};


std::shared_ptr<State> Config::sample_initial_state() {
    return std::make_shared<State>(shared_from_this());
}


struct Action : std::enable_shared_from_this<Action> {
    std::shared_ptr<State> state;
    unsigned column;

    Action(std::shared_ptr<State> const& state, unsigned column) :
        state(state),
        column(column)
    {}

    std::shared_ptr<State> sample_next_state() const {
        std::shared_ptr<State> next_state = std::make_shared<State>(*state);
        next_state->play_at(column);
        return next_state;
    }
};


}
}


#endif
