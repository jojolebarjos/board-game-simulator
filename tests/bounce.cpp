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

}
