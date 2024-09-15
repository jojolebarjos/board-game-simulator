#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "game/bounce.hpp"


using namespace game;
using namespace game::bounce;


TEST_CASE("Sanity checks on small board") {

    tensor<int8_t, -1, -1> initial_grid(6, 3);
    initial_grid.fill(0);
    initial_grid[1][0] = initial_grid[4][0] = 1;
    initial_grid[1][1] = initial_grid[4][1] = 2;
    initial_grid[1][2] = initial_grid[4][2] = 3;

    auto config = std::make_shared<Config>(initial_grid);

    // Initialization
    auto state = config->sample_initial_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);
    CHECK(state->get_grid() == initial_grid);
    CHECK(state->get_actions().size() == 8);
    CHECK(state->get_actions_at({ 0, 1 }).size() == 3);
    CHECK(state->get_actions_at({ 1, 1 }).size() == 3);
    CHECK(state->get_actions_at({ 2, 1 }).size() == 2);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 1
    state = state->get_action_at({ 1, 1 }, { 0, 2 })->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 1);

    // Turn 2
    state = state->get_action_at({ 2, 4 }, { 0, 3 })->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);

    // Turn 3
    state = state->get_action_at({ 2, 1 }, { 2, 2 })->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 1);

    // Turn 4
    auto foo = state->get_actions();
    state = state->get_action_at({ 0, 4 }, { 0, 0 })->sample_next_state();
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
