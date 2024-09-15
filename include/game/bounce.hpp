#ifndef GAME_BOUNCE_HPP
#define GAME_BOUNCE_HPP


#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include "./tensor.hpp"


namespace game {
namespace bounce {


/*
  The "playable" zone is a 7x6 grid; two additional rows act as goals.
  The origin is the lower-left corner.
  If a player cannot move, they lose; if both are blocked, this is a draw.

  - - - - - -
  1 2 3 3 2 1
  . . . . . .
  . . . . . .
  . . . . . .
  . . . . . .
  . . . . . .
  1 2 3 3 2 1
  - - - - - -
*/


typedef tensor<int, 2> Coordinate;
typedef tensor<int8_t, -1, -1> Grid;


struct Move {
	Coordinate source;
	Coordinate target;

	constexpr auto operator<=>(Move const& right) const noexcept {
		if (auto cmp = (source[0] <=> right.source[0]); cmp != 0) return cmp;
		if (auto cmp = (source[1] <=> right.source[1]); cmp != 0) return cmp;
		if (auto cmp = (target[0] <=> right.target[0]); cmp != 0) return cmp;
		return target[1] <=> right.target[1];
	}
};


struct Walk {
	Grid grid;
	std::set<Move> moves;

	Walk(Grid const& grid) : grid(grid), source() {}

	void collect(int x, int y, int dy) {
		int value = grid[y][x];
		if (value > 0) {
			source = Coordinate{ x, y };
			grid[y][x] = 0;
			recurse(x, y, 0, dy, value);
			grid[y][x] = value;
		}
	}

private:

	Coordinate source;

	void recurse(int x, int y, int dx, int dy, int remaining) {
		int height = grid.shape()[0];
		int width = grid.shape()[1];

		// Up
		if (dy >= 0) {
			if (y < height - 1) {
				int value = grid[y + 1][x];

				// Empty space
				if (value == 0) {
					if (remaining == 1)
						visit(x, y + 1);
					else
						recurse(x, y + 1, 0, dy, remaining - 1);
				}

				// Bounce
				else if (value > 0 && remaining == 1) {
					grid[y + 1][x] = -1;
					recurse(x, y + 1, 0, dy, value);
					grid[y + 1][x] = value;
				}
			}
		}

		// Down
		else {
			if (y > 0) {
				int value = grid[y - 1][x];

				// Empty space
				if (value == 0) {
					if (remaining == 1)
						visit(x, y - 1);
					else
						recurse(x, y - 1, 0, dy, remaining - 1);
				}

				// Bounce
				else if (value > 0 && remaining == 1) {
					grid[y - 1][x] = -1;
					recurse(x, y - 1, 0, dy, value);
					grid[y - 1][x] = value;
				}
			}
		}

		// Cannot move horizontally on first/last row
		if (y <= 0 || y >= height - 1)
			return;

		// Left
		if (x > 0 && dx <= 0) {
			int value = grid[y][x - 1];

			// Empty space
			if (value == 0) {
				if (remaining == 1)
					visit(x - 1, y);
				else
					recurse(x - 1, y, -1, dy, remaining - 1);
			}

			// Bounce
			else if (value > 0 && remaining == 1) {
				grid[y][x - 1] = -1;
				recurse(x - 1, y, 0, dy, value);
				grid[y][x - 1] = value;
			}
		}

		// Right
		if (x < width - 1 && dx >= 0) {
			int value = grid[y][x + 1];

			// Empty space
			if (value == 0) {
				if (remaining == 1)
					visit(x + 1, y);
				else
					recurse(x + 1, y, 1, dy, remaining - 1);
			}

			// Bounce
			else if (value > 0 && remaining == 1) {
				grid[y][x + 1] = -1;
				recurse(x + 1, y, 0, dy, value);
				grid[y][x + 1] = value;
			}
		}
	}

	void visit(int x, int y) {
		Coordinate target = { x, y };
		Move move = { source, target };
		moves.insert(move);
	}
};


struct Board {
	Grid grid;

	constexpr Board(Grid grid) : grid(grid) {}

	constexpr int get_height() const {
		return grid.shape()[0];
	}

	constexpr int get_width() const {
		return grid.shape()[1];
	}

	constexpr bool is_row_empty(int row) const {
		int height = get_height();
		int width = get_width();
		if (row >= 0 && row < height)
			for (int column = 0; column < width; ++column)
				if (grid[row][column] > 0)
					return false;
		return true;
	}

	constexpr int get_bottom_row() const {
		int height = get_height();
		for (int row = 0; row < height; ++row)
			if (!is_row_empty(row))
				return row;
		return -1;
	}

	constexpr int get_top_row() const {
		int height = get_height();
		for (int row = height - 1; row >= 0; --row)
			if (!is_row_empty(row))
				return row;
		return -1;
	}

	constexpr int get_row(int player) const {
		switch (player) {
		case 0:
			return get_bottom_row();
		case 1:
			return get_top_row();
		default:
			return -1;
		}
	}

	constexpr int get_direction(int player) const {
		switch (player) {
		case 0:
			return 1;
		case 1:
			return -1;
		default:
			return 0;
		}
	}

	std::set<Move> get_moves(int player) const {
		int width = get_width();
		int y = get_row(player);
		int dy = get_direction(player);
		Walk walk(grid);
		if (y >= 0)
			for (int x = 0; x < width; ++x)
				walk.collect(x, y, dy);
		return walk.moves;
	}

	std::set<Move> get_moves_at(int player, Coordinate source) const {
		int width = get_width();
		int x = source[0];
		int y = source[1];
		int dy = get_direction(player);
		Walk walk(grid);
		if (y == get_row(player) && x >= 0 && x < width)
			walk.collect(x, y, dy);
		return walk.moves;
	}

	bool can_play(int player) const {
		return !get_moves(player).empty();
	}

	constexpr void apply(Move const& move) {
		int value = grid[move.source[1]][move.source[0]];
		grid[move.source[1]][move.source[0]] = 0;
		grid[move.target[1]][move.target[0]] = value;
	}
};


class Config;
class State;
class Action;


struct Config : std::enable_shared_from_this<Config> {
	static constexpr int num_players = 2;
	Board board;

	Config(Grid const& grid) : board(grid) {
		if (board.get_bottom_row() == 0 || board.get_top_row() == board.get_height() - 1)
			throw std::runtime_error("bottom- and top-rows must be empty");
		// TODO check a bit more?
	}

	auto get_grid() const {
		return board.grid;
	}

	std::shared_ptr<State> sample_initial_state();
};


struct State : std::enable_shared_from_this<State> {
	std::shared_ptr<Config> config;
	Board board;
	int8_t player;
	int8_t winner;

	State(std::shared_ptr<Config> config) :
		config(config),
		board(config->board),
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

	std::vector<std::shared_ptr<Action>> get_actions();
	std::vector<std::shared_ptr<Action>> get_actions_at(Coordinate const& source);
	std::shared_ptr<Action> get_action_at(Coordinate const& source, Coordinate const& target);
};


std::shared_ptr<State> Config::sample_initial_state() {
	return std::make_shared<State>(shared_from_this());
}


struct Action : std::enable_shared_from_this<Action> {
	std::shared_ptr<State> state;
	Move move;

	Action(std::shared_ptr<State> state, Move const& move) : state(state), move(move) {}

	Coordinate get_source() const {
		return move.source;
	}

	Coordinate get_target() const {
		return move.target;
	}

	std::shared_ptr<State> sample_next_state() const {
		std::shared_ptr<State> next_state = std::make_shared<State>(*state);
		next_state->apply(*this);
		return next_state;
	}
};


void State::apply(Action const& action) {

	// Move piece
	board.apply(action.move);

	// Check for victory
	int y = action.move.target[1];
	if (y == 0 || y == board.get_height() - 1) {
		winner = player;
		player = -1;
		return;
	}

	// If next player cannot play, they lose
	// However, if the other cannot either, this is a draw
	player = player ? 0 : 1;
	if (!board.can_play(player)) {
		player = player ? 0 : 1;
		if (board.can_play(player))
			winner = player;
		player = -1;
	}
}


std::vector<std::shared_ptr<Action>> State::get_actions() {
	std::vector<std::shared_ptr<Action>> result;
	for (Move const& move : board.get_moves(player))
		result.push_back(std::make_shared<Action>(shared_from_this(), move));
	return result;
}


std::vector<std::shared_ptr<Action>> State::get_actions_at(Coordinate const& source) {
	std::vector<std::shared_ptr<Action>> result;
	for (Move const& move : board.get_moves_at(player, source))
		result.push_back(std::make_shared<Action>(shared_from_this(), move));
	return result;
}


std::shared_ptr<Action> State::get_action_at(Coordinate const& source, Coordinate const& target) {
	Move move = { source, target };
	std::set<Move> moves = board.get_moves_at(player, source);
	if (!moves.contains(move))
		throw std::runtime_error("invalid move");
	return std::make_shared<Action>(shared_from_this(), move);
}


}
}


#endif
