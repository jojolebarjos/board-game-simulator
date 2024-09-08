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
    CHECK(state->grid == tensor<int8_t, 2, 3> { -1, -1, -1, -1, -1, -1 });
    CHECK(state->actions().size() == 3);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 1
    state = state->action_at(1)->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 1);
    CHECK(state->grid == tensor<int8_t, 2, 3> { -1, 0, -1, -1, -1, -1 });
    CHECK(state->actions().size() == 3);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 2
    state = state->action_at(1)->sample_next_state();
    CHECK(!state->has_ended());
    CHECK(state->get_player() == 0);
    CHECK(state->grid == tensor<int8_t, 2, 3> { -1, 0, -1, -1, 1, -1 });
    CHECK(state->actions().size() == 2);
    CHECK(state->get_reward() == tensor<float, 2> { 0.0f, 0.0f });

    // Turn 3
    state = state->action_at(2)->sample_next_state();
    CHECK(state->has_ended());
    CHECK(state->get_player() == -1);
    CHECK(state->grid == tensor<int8_t, 2, 3> { -1, 0, 0, -1, 1, -1 });
    CHECK(state->actions().size() == 0);
    CHECK(state->get_reward() == tensor<float, 2> { 1.0f, -1.0f });
}
