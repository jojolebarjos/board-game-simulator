#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "game/bounce.hpp"


TEST_CASE("Sanity checks") {

    using Traits = game::bounce::Traits;
    using State = Traits::State;
    using Action = Traits::Action;
    using Grid = decltype(State::grid);

    State state;
    std::vector<Action> actions;

    Traits::initialize(state);

    CHECK(!Traits::has_ended(state));

    // TODO ...
}


TEST_CASE("Check blocked states") {

    using Traits = game::bounce::Traits;
    using State = Traits::State;
    using Grid = decltype(State::grid);

    State state;
    Traits::initialize(state);

    state.grid = Grid {
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };
    CHECK(!state.can_play());

    state.grid = Grid{
        0, 0, 0, 0, 0, 0,
        3, 3, 3, 3, 0, 3,
        3, 3, 3, 3, 3, 3,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };
    CHECK(!state.can_play());
    state.player = 1;
    CHECK(state.can_play());
}


TEST_CASE("Check draw") {

    using Traits = game::bounce::Traits;
    using State = Traits::State;
    using Action = Traits::Action;
    using Grid = decltype(State::grid);

    State state;
    std::vector<Action> actions;

    Traits::from_json(
        state,
        {
            {"grid", {
                {0, 0, 3, 0, 0, 0},
                {0, 0, 0, 0, 0, 0},
                {0, 0, 0, 0, 0, 0},
                {3, 0, 0, 0, 3, 0},
                {3, 3, 3, 3, 3, 3},
                {2, 2, 2, 2, 2, 2},
                {0, 0, 0, 0, 0, 0},
            }},
            {"player", 0},
            {"winner", -1}
        }
    );

    Action action;
    Traits::from_json(
        state,
        action,
        {
            {"from", {{"x", 2}, {"y", 0}}},
            {"to", {{"x", 2}, {"y", 3}}},
        }
    );

    Traits::apply(state, action);

    CHECK(state.player < 0);
    CHECK(state.winner < 0);

    nlohmann::json j = Traits::to_json(state);
    CHECK(j.size());
}
