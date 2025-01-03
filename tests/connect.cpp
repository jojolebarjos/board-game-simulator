#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "game/connect.hpp"


using namespace game;
using namespace game::connect;


TEST_CASE("Sanity checks on small board") {

    auto config = std::make_shared<Config>(2, 3, 2);

    // Initialization
    auto state = config->sample_initial_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);
    CHECK(state->get_grid() == tensor<int8_t, 2, 3> { -1, -1, -1, -1, -1, -1 });
    CHECK(state->get_actions().size() == 3);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 1
    state = state->get_action_at(1)->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 1);
    CHECK(state->get_grid() == tensor<int8_t, 2, 3> { -1, 0, -1, -1, -1, -1 });
    CHECK(state->get_actions().size() == 3);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 2
    state = state->get_action_at(1)->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);
    CHECK(state->get_grid() == tensor<int8_t, 2, 3> { -1, 0, -1, -1, 1, -1 });
    CHECK(state->get_actions().size() == 2);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 3
    state = state->get_action_at(2)->sample_next_state();
    CHECK(state->has_ended());
    CHECK(state->get_player() == -1);
    CHECK(state->get_grid() == tensor<int8_t, 2, 3> { -1, 0, 0, -1, 1, -1 });
    CHECK(state->get_actions().size() == 0);
    CHECK(state->get_reward() == tensor<float, 2> { 1.0f, -1.0f });
}


TEST_CASE("Hash and equal") {
    auto config = std::make_shared<Config>(6, 7, 4);

    auto initial_state = config->sample_initial_state();

    auto state_a = initial_state
        ->get_action_at(1)->sample_next_state()
        ->get_action_at(2)->sample_next_state()
        ->get_action_at(3)->sample_next_state()
        ->get_action_at(1)->sample_next_state();

    auto state_b = initial_state
        ->get_action_at(3)->sample_next_state()
        ->get_action_at(2)->sample_next_state()
        ->get_action_at(1)->sample_next_state()
        ->get_action_at(1)->sample_next_state();

    CHECK(*initial_state < *state_b);

    CHECK(*state_a == *state_b);
    CHECK(hash_value(state_a) == hash_value(state_b));

    CHECK(*state_a->get_action_at(0) == *state_b->get_action_at(0));
    CHECK(hash_value(state_a->get_action_at(0)) == hash_value(state_b->get_action_at(0)));

    CHECK(*state_a->config == *state_b->config);
    CHECK(hash_value(state_a->config) == hash_value(state_b->config));

    CHECK(hash_value(state_a) != hash_value(initial_state));
    CHECK(hash_value(state_a->get_action_at(0)) != hash_value(initial_state->get_action_at(0)));
}


TEST_CASE("JSON") {

    auto config = std::make_shared<Config>(2, 3, 2);

    CHECK(config->to_json() == nlohmann::json { { "height", 2 }, { "width", 3 }, { "count", 2 } });
    CHECK(*Config::from_json(config->to_json()) == *config);

    auto state = config->sample_initial_state()->get_action_at(2)->sample_next_state();
    CHECK(state->to_json() == nlohmann::json{
        { "grid", { {-1, -1, 0 }, { -1, -1, -1 } } },
        { "player", 1 }
    });
    CHECK(*State::from_json(state->to_json(), config) == *state);

    auto action = state->get_action_at(1);
    CHECK(action->to_json() == nlohmann::json{
        { "column", 1 }
    });
    CHECK(*Action::from_json(action->to_json(), state) == *action);
}
