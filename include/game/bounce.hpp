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
  The origin of the 7x6 grid is the lower-left corner:

  1 2 3 3 2 1
  . . . . . .
  . . . . . .
  . . . . . .
  . . . . . .
  . . . . . .
  1 2 3 3 2 1
*/


// TODO could probably make this size a template
constexpr unsigned HEIGHT = 7;
constexpr unsigned WIDTH = 6;


typedef tensor<int8_t, 2> Coordinate;



struct Walk {
	tensor<int8_t, HEIGHT, WIDTH> grid;
	std::set<Coordinate> moves;

	Walk(tensor<int8_t, HEIGHT, WIDTH> const& grid) : grid(grid) {}

	void collect(int x, int y, int dy) {
		moves.clear();
		int value = grid[y][x];
		if (value > 0) {
			grid[y][x] = 0;
			recurse(x, y, 0, dy, value);
			grid[y][x] = value;
		}
	}

private:

	void add(int x, int y) {
		Coordinate c = {x, y};
		moves.insert(c);
	}

	void recurse(int x, int y, int dx, int dy, int remaining) {

		// Up
		if (dy >= 0) {
			if (y < HEIGHT - 1) {
				int value = grid[y + 1][x];

				// Empty space
				if (value == 0) {
					if (remaining == 1)
						add(x, y + 1);
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

			// Winning move
			else if (remaining == 1) {
				add(0, HEIGHT);
			}
		}

		// Down
		else {
			if (y > 0) {
				int value = grid[y - 1][x];

				// Empty space
				if (value == 0) {
					if (remaining == 1)
						add(x, y - 1);
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

			// Winning move
			else if (remaining == 1) {
				add(0, -1);
			}
		}

		// Left
		if (x > 0 && dx <= 0) {
			int value = grid[y][x - 1];

			// Empty space
			if (value == 0) {
				if (remaining == 1)
					add(x - 1, y);
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
		if (x < WIDTH - 1 && dx >= 0) {
			int value = grid[y][x + 1];

			// Empty space
			if (value == 0) {
				if (remaining == 1)
					add(x + 1, y);
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

		// Special case, play wins
		if (action.to[0] < 0 || action.to[0] >= HEIGHT) {
			winner = player;
			return;
		}

		// Place piece
		grid[action.to[1]][action.to[0]] = value;

		// Game has not ended
		player = player ? 0 : 1;
	}

	constexpr bool is_row_empty(int y) const {
		if (y >= 0 && y < HEIGHT)
			for (unsigned x = 0; x < WIDTH; ++x)
				if (grid[y][x] > 0)
					return false;
		return true;
	}

	void get_actions(std::vector<Action>& actions) const {

		// If game has not ended
		if (player >= 0) {

			// Player 0 moves upward, while player 1 moves downward
			int8_t y = 0;
			int8_t dy = 1;
			if (player == 1) {
				y = HEIGHT - 1;
				dy = -1;
			}

			// Can only play pieces on the closest row
			while (is_row_empty(y))
				y += dy;

			// Collect moves
			Walk walk(grid);
			size_t count = actions.size();
			for (int8_t x = 0; x < WIDTH; ++x)
				if (grid[y][x] > 0) {
					walk.collect(x, y, dy);
					for (Coordinate const& to : walk.moves)
						actions.emplace_back(Coordinate{ x, y }, to);
				}

			// In rare cases, player might be blocked, but the opponent shouldn't, so force skip
			// TODO double check that... or should we say that not being to move is instant loss?
			if (actions.size() == count)
				actions.emplace_back(Coordinate{ 0, 0 }, Coordinate{ 0, 0 });
		}
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
		j.at("grid").get_to(state.grid);
		j.at("player").get_to(state.player);
		j.at("winner").get_to(state.winner);
	}

	static void from_json(State const& state, Action& action, nlohmann::json const& j) {
		j.at("from").at("x").get_to(action.from[0]);
		j.at("from").at("y").get_to(action.from[1]);
		j.at("to").at("x").get_to(action.to[0]);
		j.at("to").at("y").get_to(action.to[1]);
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
		return game::hash(state.grid, state.player);
	}

	static constexpr size_t hash(State const& state, Action const& action) noexcept {
		return game::hash(state.grid, state.player, action.from, action.to);
	}
};


}
}


#endif
