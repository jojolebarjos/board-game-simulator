#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

#include "game/connect.hpp"


template <typename Traits>
void run(float duration) {
    using State = Traits::State;
    using Action = Traits::Action;

    std::random_device device;
    unsigned seed = device();

    std::default_random_engine engine(seed);

    State state;
    std::vector<Action> actions;

    size_t action_count = 0;
    size_t game_count = 0;

    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::duration<float>(duration);

    while (std::chrono::steady_clock::now() < end) {

        Traits::initialize(state);

        while (!Traits::has_ended(state)) {

            actions.clear();
            Traits::get_actions(state, actions);

            if (actions.empty())
                break;

            std::uniform_int_distribution<size_t> distribution(0, actions.size() - 1);
            size_t i = distribution(engine);

            Traits::apply(state, actions[i]);

            action_count += 1;
        }

        game_count += 1;
    }

    std::chrono::duration<float> elapsed = std::chrono::steady_clock::now() - start;
    duration = elapsed.count();

    printf("Actions: %zu\n", action_count);
    printf("Games: %zu\n", game_count);
    printf("Average actions per game: %.02f\n", (float)action_count / game_count);
    printf("Time per action: %.06f us\n", 1e6f * duration / action_count);
    printf("Time per game: %.06f us\n", 1e6f * duration / game_count);
    // TODO also report rewards
}


int main(int argc, char* argv[]) {
    run<game::connect::Traits<6, 7, 4>>(5.0f);
    return 0;
}
