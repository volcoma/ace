#include "uuid.h"

namespace ace
{

namespace
{
auto get_generator() -> hpp::uuid_random_generator&
{
    static thread_local auto engine = []()
    {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        return generator;
    }();

    static thread_local hpp::uuid_random_generator gen{engine};
    return gen;
}
} // namespace

auto generate_uuid() -> hpp::uuid
{
    auto& generator = get_generator();
    return generator();
}

} // namespace ace
