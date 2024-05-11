#ifndef GAME_BOUNCE_HPP
#define GAME_BOUNCE_HPP


#include <algorithm>
#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

#include "./hash.hpp"
#include "./tensor.hpp"


namespace game {
namespace bounce {


/*
  The origin of the 7x6 grid is the lower-left corner.
  Special "winning" rows are at y=-1 and y=7, with x=0 (centered for visual purpose).
  If a player cannot move, they lose.

       .
  1 2 3 3 2 1
  . . . . . .
  . . . . . .
  . . . . . .
  . . . . . .
  . . . . . .
  1 2 3 3 2 1
       .
*/


// TODO could probably make this size a template
constexpr unsigned HEIGHT = 7;
constexpr unsigned WIDTH = 6;


typedef tensor<int8_t, 2> Coordinate;


struct Walk {
	tensor<int8_t, HEIGHT, WIDTH> grid;

	Walk(tensor<int8_t, HEIGHT, WIDTH> const& grid) : grid(grid) {}

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

		// Up
		if (dy >= 0) {
			if (y < HEIGHT - 1) {
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
				visitor(0, HEIGHT);
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
				visitor(0, -1);
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
		if (x < WIDTH - 1 && dx >= 0) {
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


struct Action {
	Coordinate from;
	Coordinate to;

	// TODO swap

	constexpr auto operator<=>(Action const& other) const noexcept = default;
};


struct State {
	tensor<int8_t, HEIGHT, WIDTH> grid;
	int8_t player;
	int8_t winner;

	constexpr void initialize() {
		for (unsigned i = 0; i < HEIGHT; ++i) {
			for (unsigned j = 0; j < WIDTH; ++j) {
				grid[i][j] = 0;
			}
		}
		grid[0][0] = grid[HEIGHT - 1][0] = 1;
		grid[0][1] = grid[HEIGHT - 1][1] = 2;
		grid[0][2] = grid[HEIGHT - 1][2] = 3;
		grid[0][3] = grid[HEIGHT - 1][3] = 3;
		grid[0][4] = grid[HEIGHT - 1][4] = 2;
		grid[0][5] = grid[HEIGHT - 1][5] = 1;
		player = 0;
		winner = -1;
	}

	constexpr void swap() {
		for (unsigned i = 0; i < HEIGHT / 2; ++i) {
			for (unsigned j = 0; j < WIDTH; ++j) {
				std::swap(grid[i][j], grid[HEIGHT - i - 1][j]);
			}
		}
		if (player >= 0)
			player = 1 - player;
		else if (winner >= 0)
			winner = 1 - winner;
	}

	constexpr void mirror() {
		for (unsigned i = 0; i < HEIGHT; ++i) {
			for (unsigned j = 0; j < WIDTH / 2; ++j) {
				std::swap(grid[i][j], grid[i][WIDTH - j - 1]);
			}
		}
	}

	constexpr void apply(Action const& action) {

		// Remove piece
		uint8_t value = grid[action.from[1]][action.from[0]];
		grid[action.from[1]][action.from[0]] = 0;

		// Special case, player wins
		if (action.to[1] < 0 || action.to[1] >= HEIGHT) {
			winner = player;
			player = -1;
			return;
		}

		// Place piece
		grid[action.to[1]][action.to[0]] = value;

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

	constexpr bool is_row_empty(int y) const {
		if (y >= 0 && y < HEIGHT)
			for (unsigned x = 0; x < WIDTH; ++x)
				if (grid[y][x] > 0)
					return false;
		return true;
	}

	template <typename Visitor>
	void visit_actions(Visitor visitor) const {

		// Check whether game has ended
		if (player < 0 || player > 1)
			return;

		// Player 0 moves upward, while player 1 moves downward
		int8_t y = 0;
		int8_t dy = 1;
		int8_t limit = HEIGHT;
		if (player == 1) {
			y = HEIGHT - 1;
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
		for (int8_t x = 0; x < WIDTH; ++x)
			if (grid[y][x] > 0) {
				auto subvisitor = [&](int x_, int y_) {
					Action action = { x, y, (int8_t)x_, (int8_t)y_ };
					visitor(action);
				};
				walk(subvisitor, x, y, dy);
			}
	}

	bool can_play() const {
		auto visitor = [](Action const&) { throw 0; };
		try {
			visit_actions(visitor);
		}
		catch (int) {
			return true;
		}
		return false;
	}

	void get_actions(std::vector<Action>& actions) const {
		std::set<Action> buffer;
		auto visitor = [&](Action const& action) {
			buffer.insert(action);
		};
		visit_actions(visitor);
		actions.insert(actions.end(), buffer.begin(), buffer.end());
	}

	void validate() const {

		// Forbid invalid values in grid
		for (int y = 0; y < HEIGHT; ++y)
			for (int x = 0; x < WIDTH; ++x)
				if (grid[y][x] < 0)
					throw std::runtime_error("values must be non-negative");

		// Current player must be able to move
		if (player >= 0 && !can_play())
			throw std::runtime_error("player has no valid move");
	}

	constexpr auto operator<=>(State const& other) const = default;
};


struct Traits {
	// TODO reward object?
	using State = game::bounce::State;
	using Action = game::bounce::Action;

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

	static constexpr std::tuple<tensor<int8_t, HEIGHT, WIDTH>> get_tensors(State const& state) {
		return { state.grid };
	}

	// TODO add data augmentation helper (i.e. mirror)

	static void get_actions(State const& state, std::vector<Action>& actions) {
		state.get_actions(actions);
	}

	static constexpr void apply(State& state, Action const& action) {
		state.apply(action);
	}

	static nlohmann::json to_json(State const& state) {
		// TODO maybe player/winner should be null?
		return {
			{"grid", state.grid},
			{"player", state.player},
			{"winner", state.winner}
		};
	}

	static nlohmann::json to_json(State const& state, Action const& action) {
		return {
			{"from", {
				{"x", action.from[0]},
				{"y", action.from[1]}
			}},
			{"to", {
				{"x", action.to[0]},
				{"y", action.to[1]}
			}}
		};
	}

	static void from_json(State& state, nlohmann::json const& j) {

		// Decode JSON object
		j.at("grid").get_to(state.grid);
		j.at("player").get_to(state.player);
		j.at("winner").get_to(state.winner);

		// Validate state
		state.validate();
	}

	static void from_json(State const& state, Action& action, nlohmann::json const& j) {

		// Decode JSON object
		j.at("from").at("x").get_to(action.from[0]);
		j.at("from").at("y").get_to(action.from[1]);
		j.at("to").at("x").get_to(action.to[0]);
		j.at("to").at("y").get_to(action.to[1]);

		// Ensure action is valid
		std::vector<Action> actions;
		state.get_actions(actions);
		if (std::find(actions.begin(), actions.end(), action) == actions.end())
			throw std::runtime_error("invalid action");
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
		return game::hash(state.grid, state.player);
	}

	static constexpr size_t hash(State const& state, Action const& action) noexcept {
		return game::hash(state.grid, state.player, action.from, action.to);
	}
};


}
}


#endif
