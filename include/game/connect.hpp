#ifndef GAME_CONNECT_HPP
#define GAME_CONNECT_HPP


#include <memory>
#include <stdexcept>
#include <vector>

#include <nlohmann/json.hpp>

#include "./comparison.hpp"
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


struct Board {
    tensor<int8_t, -1, -1> grid;

    constexpr Board(int height, int width) : grid(height, width) {
        grid.fill(-1);
    }

    constexpr int height() const {
        return grid.shape()[0];
    }

    constexpr int width() const {
        return grid.shape()[1];
    }

    constexpr bool is_full() const {
        int h = height();
        int w = width();
        for (int column = 0; column < w; ++column)
            if (grid[h - 1][column] < 0)
                return false;
        return true;
    }

    constexpr bool can_play_at(int column) const {
        return column >= 0 && column < width() && grid[height() - 1][column] < 0;
    }

    constexpr int play_at(int column, int player) {
        int h = height();
        int w = width();
        if (column >= 0 && column < w)
            for (int row = 0; row < h; ++row)
                if (grid[row][column] < 0) {
                    grid[row][column] = player;
                    return row;
                }
        return -1;
    }

    constexpr int count_at(int row, int column) const {
        int player = grid[row][column];
        int h = height();
        int w = width();

        int u = 1;
        for (int j = column; j > 0 && grid[row][--j] == player; ++u);
        for (int j = column; j < w - 1 && grid[row][++j] == player; ++u);

        int v = 1;
        for (int i = row; i > 0 && grid[--i][column] == player; ++v);
        for (int i = row; i < h - 1 && grid[++i][column] == player; ++v);

        int a = 1;
        for (int i = row, j_ = column; i > 0 && j_ > 0 && grid[--i][--j_] == player; ++a);
        for (int i = row, j_ = column; i < h - 1 && j_ < w - 1 && grid[++i][++j_] == player; ++a);

        int b = 1;
        for (int i = row, j = column; i > 0 && j < w - 1 && grid[--i][++j] == player; ++b);
        for (int i = row, j = column; i < h - 1 && j > 0 && grid[++i][--j] == player; ++b);

        return std::max({ u, v, a, b });
    }

    constexpr auto operator<=>(Board const& right) const {
        return std::lexicographical_compare_three_way(
            grid.data(), grid.data() + grid.size(),
            right.grid.data(), right.grid.data() + right.grid.size()
        );
    }
};


struct Config;
struct State;
struct Action;


struct Config : std::enable_shared_from_this<Config>, Comparable<Config> {
    using State = State;
    using Action = Action;

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

    auto get_identity_tuple() const {
        return std::tie(height, width, count);
    }

    nlohmann::json to_json() const {
        return {
            { "height", height },
            { "width", width },
            { "count", count }
        };
    }

    static std::shared_ptr<Config> from_json(nlohmann::json const& j) {
        int height = 0;
        int width = 0;
        int count = 0;
        j.at("height").get_to(height);
        j.at("width").get_to(width);
        j.at("count").get_to(count);
        return std::make_shared<Config>(height, width, count);
    }
};


struct State : std::enable_shared_from_this<State>, Comparable<State> {
    using Config = Config;
    using Action = Action;

    std::shared_ptr<Config> config;
    Board board;
    int8_t player;
    int8_t winner;

    State(std::shared_ptr<Config> const& config) :
        config(config),
        board(config->height, config->width),
        player(0),
        winner(-1)
    {}

    auto get_grid() const {
        return board.grid;
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

    void apply(Action const& action);

    std::shared_ptr<Action> get_action_at(int column) {
        if (player < 0 || !board.can_play_at(column))
            throw std::runtime_error("invalid move");
        return std::make_shared<Action>(shared_from_this(), column);
    }

    std::vector<std::shared_ptr<Action>> get_actions() {
        std::vector<std::shared_ptr<Action>> result;
        if (player >= 0)
            for (unsigned column = 0; column < config->width; ++column)
                if (board.can_play_at(column))
                    result.push_back(get_action_at(column));
        return result;
    }

    auto get_identity_tuple() const {
        return std::tie(board.grid, player);
    }

    nlohmann::json to_json() const {
        return {
            { "grid", board.grid },
            { "player", player }
        };
    }

    static std::shared_ptr<State> from_json(nlohmann::json const& j, std::shared_ptr<Config> const& config) {
        auto state = std::make_shared<State>(config);
        j.at("grid").get_to(state->board.grid);
        j.at("player").get_to(state->player);
        // TODO set winner accordingly
        // TODO check player
        // TODO check that board matches configuration
        return state;
    }
};


std::shared_ptr<State> Config::sample_initial_state() {
    return std::make_shared<State>(shared_from_this());
}


struct Action : std::enable_shared_from_this<Action>, Comparable<Action> {
    using Config = Config;
    using State = State;

    std::shared_ptr<State> state;
    unsigned column;

    Action(std::shared_ptr<State> const& state, unsigned column) :
        state(state),
        column(column)
    {}

    std::shared_ptr<State> sample_next_state() const {
        std::shared_ptr<State> next_state = std::make_shared<State>(*state);
        next_state->apply(*this);
        return next_state;
    }

    auto get_identity_tuple() const {
        return std::tie(state->board.grid, state->player, column);
    }

    nlohmann::json to_json() const {
        return {
            { "column", column }
        };
    }

    static std::shared_ptr<Action> from_json(nlohmann::json const& j, std::shared_ptr<State> const& state) {
        unsigned column = 0;
        j.at("column").get_to(column);
        auto action = std::make_shared<Action>(state, column);
        // TODO check that column is valid
        return action;
    }
};


void State::apply(Action const& action) {
    int row = board.play_at(action.column, player);
    if (row < 0)
        throw std::runtime_error("invalid move");
    if (board.count_at(row, action.column) >= config->count) {
        winner = player;
        player = -1;
        return;
    }
    if (board.is_full()) {
        player = -1;
        return;
    }
    player = player ? 0 : 1;
}


}
}


#endif
