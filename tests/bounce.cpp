#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "game/bounce.hpp"


using namespace game;
using namespace game::bounce;


TEST_CASE("Sanity checks on small board") {

    tensor<int8_t, -1, -1> initial_grid(4, 3);
    initial_grid.fill(0);
    initial_grid[0][0] = initial_grid[3][0] = 1;
    initial_grid[0][1] = initial_grid[3][1] = 2;
    initial_grid[0][2] = initial_grid[3][2] = 3;

    auto config = std::make_shared<Config>(initial_grid);

    // Initialization
    auto state = config->sample_initial_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);
    CHECK(state->grid == initial_grid);
    CHECK(state->actions().size() == 8);
    CHECK(state->actions_at({ 0, 0 }).size() == 3);
    CHECK(state->actions_at({ 1, 0 }).size() == 3);
    CHECK(state->actions_at({ 2, 0 }).size() == 2);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 1
    state = state->action_at({ 1, 0 }, { 0, 1 })->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 1);

    // Turn 2
    state = state->action_at({ 2, 3 }, { 0, 2 })->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);

    // Turn 3
    state = state->action_at({ 2, 0 }, { 2, 1 })->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 1);

    // Turn 4
    state = state->action_at({ 0, 3 }, { 0, -1 })->sample_next_state();
    CHECK(state->has_ended());
    CHECK(state->get_reward() == tensor<float, 2> { -1.0f, 1.0f });
}


TEST_CASE("Check blocked states") {

    // TODO empty cannot be played
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0

    // TODO player 1 cannot play, but player 0 can
    //   0, 0, 0, 0, 0, 0
    //   3, 3, 3, 3, 0, 3
    //   3, 3, 3, 3, 3, 3
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0

    // TODO player 1 can play (2, 6) -> (2, 3), and then it is draw
    //   0, 0, 3, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   0, 0, 0, 0, 0, 0
    //   3, 0, 0, 0, 3, 0
    //   3, 3, 3, 3, 3, 3
    //   2, 2, 2, 2, 2, 2
    //   0, 0, 0, 0, 0, 0
}
