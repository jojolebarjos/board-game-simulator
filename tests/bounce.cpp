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


TEST_CASE("Hash and equal") {
    tensor<int8_t, -1, -1> grid(9, 6);
    grid.storage = std::vector<int8_t>{
        0, 0, 0, 0, 0, 0,
        1, 2, 3, 3, 2, 1,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        1, 2, 3, 3, 2, 1,
        0, 0, 0, 0, 0, 0
    };

    auto config = std::make_shared<Config>(grid);

    auto initial_state = config->sample_initial_state();

    auto state_a = initial_state
        ->get_action_at({ 0, 1 }, { 1, 3 })->sample_next_state()
        ->get_action_at({ 2, 7 }, { 3, 5 })->sample_next_state()
        ->get_action_at({ 1, 1 }, { 0, 2 })->sample_next_state()
        ->get_action_at({ 3, 7 }, { 2, 5 })->sample_next_state();

    auto state_b = initial_state
        ->get_action_at({ 1, 1 }, { 0, 2 })->sample_next_state()
        ->get_action_at({ 3, 7 }, { 2, 5 })->sample_next_state()
        ->get_action_at({ 0, 1 }, { 1, 3 })->sample_next_state()
        ->get_action_at({ 2, 7 }, { 3, 5 })->sample_next_state();

    CHECK(*state_a == *state_b);
    CHECK(hash_value(state_a) == hash_value(state_b));

    CHECK(*state_a->get_action_at({ 5, 1 }, { 5, 2 }) == *state_b->get_action_at({ 5, 1 }, { 5, 2 }));
    CHECK(hash_value(state_a->get_action_at({ 5, 1 }, { 5, 2 })) == hash_value(state_b->get_action_at({ 5, 1 }, { 5, 2 })));

    CHECK(*state_a->config == *state_b->config);
    CHECK(hash_value(state_a->config) == hash_value(state_b->config));

    CHECK(hash_value(state_a) != hash_value(initial_state));
    CHECK(hash_value(state_a->get_action_at({ 5, 1 }, { 5, 2 })) != hash_value(initial_state->get_action_at({ 5, 1 }, { 5, 2 })));
}


TEST_CASE("JSON") {

    tensor<int8_t, -1, -1> grid(6, 3);
    grid.storage = std::vector<int8_t>{
        0, 0, 0,
        1, 2, 3,
        0, 0, 0,
        0, 0, 0,
        1, 2, 3,
        0, 0, 0
    };

    auto config = std::make_shared<Config>(grid);

    CHECK(config->to_json() == nlohmann::json {
        { "grid", {
            { 0, 0, 0 },
            { 1, 2, 3 },
            { 0, 0, 0 },
            { 0, 0, 0 },
            { 1, 2, 3 },
            { 0, 0, 0 }
        } }
    });
    CHECK(*Config::from_json(config->to_json()) == *config);

    auto state = config->sample_initial_state()->get_action_at({ 2, 1 }, { 1, 3 })->sample_next_state();
    CHECK(state->to_json() == nlohmann::json{
        { "grid", {
            { 0, 0, 0 },
            { 1, 2, 0 },
            { 0, 0, 0 },
            { 0, 3, 0 },
            { 1, 2, 3 },
            { 0, 0, 0 }
        } },
        { "player", 1 }
    });
    CHECK(*State::from_json(state->to_json(), config) == *state);

    auto action = state->get_action_at({ 0, 4 }, { 0, 3 });
    CHECK(action->to_json() == nlohmann::json{
        { "source", { 0, 4 } },
        { "target", { 0, 3 } }
    });
    CHECK(*Action::from_json(action->to_json(), state) == *action);
}
