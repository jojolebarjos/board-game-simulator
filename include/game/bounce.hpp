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
  The origin of the 7x6 grid is the lower-left corner.
  Special "winning" rows are at y=-1 and y=7.
  If a player cannot move, they lose.

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


struct Move {
	int sx, sy, tx, ty;

	constexpr auto operator<=>(Move const& other) const noexcept = default;
};


struct Walk {
	tensor<int8_t, -1, -1> grid;

	Walk(tensor<int8_t, -1, -1> const& grid) : grid(grid) {}

	template <typename Visitor>
	void operator()(Visitor visitor, int x, int y, int dy) {
		int value = grid[y][x];
		if (value > 0) {
			grid[y][x] = 0;
			recurse(visitor, x, y, 0, dy, value);
			grid[y][x] = value;
		}
	}

private:

	template <typename Visitor>
	void recurse(Visitor visitor, int x, int y, int dx, int dy, int remaining) {
		int height = grid.shape()[0];
		int width = grid.shape()[1];

		// Up
		if (dy >= 0) {
			if (y < height - 1) {
				int value = grid[y + 1][x];

				// Empty space
				if (value == 0) {
					if (remaining == 1)
						visitor(x, y + 1);
					else
						recurse(visitor, x, y + 1, 0, dy, remaining - 1);
				}

				// Bounce
				else if (value > 0 && remaining == 1) {
					grid[y + 1][x] = -1;
					recurse(visitor, x, y + 1, 0, dy, value);
					grid[y + 1][x] = value;
				}
			}

			// Winning move
			else if (remaining == 1) {
				visitor(x, height);
			}
		}

		// Down
		else {
			if (y > 0) {
				int value = grid[y - 1][x];

				// Empty space
				if (value == 0) {
					if (remaining == 1)
						visitor(x, y - 1);
					else
						recurse(visitor, x, y - 1, 0, dy, remaining - 1);
				}

				// Bounce
				else if (value > 0 && remaining == 1) {
					grid[y - 1][x] = -1;
					recurse(visitor, x, y - 1, 0, dy, value);
					grid[y - 1][x] = value;
				}
			}

			// Winning move
			else if (remaining == 1) {
				visitor(x, -1);
			}
		}

		// Left
		if (x > 0 && dx <= 0) {
			int value = grid[y][x - 1];

			// Empty space
			if (value == 0) {
				if (remaining == 1)
					visitor(x - 1, y);
				else
					recurse(visitor, x - 1, y, -1, dy, remaining - 1);
			}

			// Bounce
			else if (value > 0 && remaining == 1) {
				grid[y][x - 1] = -1;
				recurse(visitor, x - 1, y, 0, dy, value);
				grid[y][x - 1] = value;
			}
		}

		// Right
		if (x < width - 1 && dx >= 0) {
			int value = grid[y][x + 1];

			// Empty space
			if (value == 0) {
				if (remaining == 1)
					visitor(x + 1, y);
				else
					recurse(visitor, x + 1, y, 1, dy, remaining - 1);
			}

			// Bounce
			else if (value > 0 && remaining == 1) {
				grid[y][x + 1] = -1;
				recurse(visitor, x + 1, y, 0, dy, value);
				grid[y][x + 1] = value;
			}
		}
	}
};


struct State;


struct Action : std::enable_shared_from_this<Action> {
	std::shared_ptr<State> state;
	tensor<int, 2> source;
	tensor<int, 2> target;

	Action(std::shared_ptr<State> state, tensor<int, 2> const& source, tensor<int, 2> const& target) :
		state(state),
		source(source),
		target(target)
	{}

	std::shared_ptr<State> sample_next_state() const;
};


struct Config : std::enable_shared_from_this<Config> {
    static constexpr int num_players = 2;
    tensor<int8_t, -1, -1> grid;

    Config(tensor<int8_t, -1, -1> const& grid) : grid(grid) {
		// TODO check a bit?
    }

    std::shared_ptr<State> sample_initial_state();
};


struct State : std::enable_shared_from_this<State> {
    std::shared_ptr<Config> config;
    tensor<int8_t, -1, -1> grid;
    int8_t player;
    int8_t winner;

    State(std::shared_ptr<Config> config) :
        config(config),
        grid(config->grid),
        player(0),
        winner(-1)
    {}

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

	void apply(Action const& action) {
		int height = grid.shape()[0];

		// Remove piece
		uint8_t value = grid[action.source[1]][action.source[0]];
		grid[action.source[1]][action.source[0]] = 0;

		// Special case, player wins
		if (action.target[1] < 0 || action.target[1] >= height) {
			winner = player;
			player = -1;
			return;
		}

		// Place piece
		grid[action.target[1]][action.target[0]] = value;

		// Game has not ended
		player = player ? 0 : 1;

		// If current player cannot play, they lose
		// However, if the other cannot either, this is a draw
		if (!can_play()) {
			player = player ? 0 : 1;
			if (can_play())
				winner = player;
			player = -1;
		}
	}

	bool is_row_empty(int y) const {
		int height = grid.shape()[0];
		int width = grid.shape()[1];
		if (y >= 0 && y < height)
			for (int x = 0; x < width; ++x)
				if (grid[y][x] > 0)
					return false;
		return true;
	}

	template <typename Visitor>
	void visit_moves(Visitor visitor) {
		int height = grid.shape()[0];
		int width = grid.shape()[1];

		// Check whether game has ended
		if (player < 0 || player > 1)
			return;

		// Player 0 moves upward, while player 1 moves downward
		int y = 0;
		int dy = 1;
		int limit = height;
		if (player == 1) {
			y = height - 1;
			dy = -1;
			limit = -1;
		}

		// Can only play pieces on the closest row
		while (is_row_empty(y)) {
			y += dy;

			// Check if board is empty
			if (y == limit)
				return;
		}

		// Enumerate moves
		Walk walk(grid);
		for (int x = 0; x < width; ++x)
			if (grid[y][x] > 0) {
				auto subvisitor = [&](int x_, int y_) {
					Move move = { x, y, x_, y_ };
					visitor(move);
				};
				walk(subvisitor, x, y, dy);
			}
	}

	bool can_play() {
		auto visitor = [](Move const&) { throw 0; };
		try {
			visit_moves(visitor);
		}
		catch (int) {
			return true;
		}
		return false;
	}

	std::set<Move> get_moves() {
		std::set<Move> moves;
		auto visitor = [&](Move const& move) {
			moves.insert(move);
			};
		visit_moves(visitor);
		return moves;
	}

	std::vector<std::shared_ptr<Action>> actions() {
		std::set<Move> moves = get_moves();
		std::vector<std::shared_ptr<Action>> result;
		for (Move const& move : moves)
			result.push_back(std::make_shared<Action>(shared_from_this(), tensor<int, 2> { move.sx, move.sy }, tensor<int, 2> { move.tx, move.ty }));
		return result;
	}

	std::vector<std::shared_ptr<Action>> actions_at(tensor<int, 2> const& source) {
		std::set<Move> moves = get_moves();
		std::vector<std::shared_ptr<Action>> result;
		for (Move const& move : moves)
			if (move.sx == source[0] && move.sy == source[1])
				result.push_back(std::make_shared<Action>(shared_from_this(), tensor<int, 2> { move.sx, move.sy }, tensor<int, 2> { move.tx, move.ty }));
		return result;
	}

	std::shared_ptr<Action> action_at(tensor<int, 2> const& source, tensor<int, 2> const& target) {
		Move move = { source[0], source[1], target[0], target[1] };
		std::set<Move> moves = get_moves();
		if (!moves.contains(move))
			throw std::runtime_error("invalid move");
		return std::make_shared<Action>(shared_from_this(), tensor<int, 2> { move.sx, move.sy }, tensor<int, 2> { move.tx, move.ty });
	}

};


std::shared_ptr<State> Config::sample_initial_state() {
    return std::make_shared<State>(shared_from_this());
}


std::shared_ptr<State> Action::sample_next_state() const {
	std::shared_ptr<State> next_state = std::make_shared<State>(*state);
	next_state->apply(*this);
	return next_state;
}


}
}


#endif
