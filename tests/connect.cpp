#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "game/connect.hpp"


TEST_CASE("Sanity checks on small board") {

    using Traits = game::connect::Traits<2, 3, 2>;
    using State = Traits::State;
    using Action = Traits::Action;
    using Grid = decltype(State::grid);

    State state;
    std::vector<Action> actions;

    Traits::initialize(state);

    // Turn 1
    CHECK(!Traits::has_ended(state));
    CHECK(Traits::get_player(state) == 0);
    CHECK(Traits::get_winner(state) == -1);
    actions.clear();
    Traits::get_actions(state, actions);
    CHECK(actions.size() == 3);
    CHECK(actions[0] == 0);
    CHECK(actions[1] == 1);
    CHECK(actions[2] == 2);
    Traits::apply(state, actions[1]);
    CHECK(state.grid == Grid{-1, 0, -1, -1, -1, -1});

    // Turn 2
    CHECK(!Traits::has_ended(state));
    CHECK(Traits::get_player(state) == 1);
    CHECK(Traits::get_winner(state) == -1);
    actions.clear();
    Traits::get_actions(state, actions);
    CHECK(actions.size() == 3);
    CHECK(actions[0] == 0);
    CHECK(actions[1] == 1);
    CHECK(actions[2] == 2);
    Traits::apply(state, actions[1]);
    CHECK(state.grid == Grid{ -1, 0, -1, -1, 1, -1 });

    // Turn 3
    CHECK(!Traits::has_ended(state));
    CHECK(Traits::get_player(state) == 0);
    CHECK(Traits::get_winner(state) == -1);
    actions.clear();
    Traits::get_actions(state, actions);
    CHECK(actions.size() == 2);
    CHECK(actions[0] == 0);
    CHECK(actions[1] == 2);
    Traits::apply(state, actions[0]);
    CHECK(state.grid == Grid{ 0, 0, -1, -1, 1, -1 });

    // End
    CHECK(Traits::has_ended(state));
    CHECK(Traits::get_winner(state) == 0);
}
