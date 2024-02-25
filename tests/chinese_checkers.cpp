#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "game/chinese_checkers.hpp"


TEST_CASE("Sanity checks on small board") {

    using Traits = game::chinese_checkers::Traits;
    using State = Traits::State;
    using Action = Traits::Action;

    State state;
    std::vector<Action> actions;

    Traits::initialize(state);

    CHECK(!Traits::has_ended(state));
}
