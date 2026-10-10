#pragma once
#include <cstdint>
#include <span>
#include <string>

namespace SkyPrompt::AddOns::SpecialEffects {

    enum EffectID : uint32_t {
        kNone = 0,
        kVinyArcs = 1,
        kTextBackground = 2,
        kSeparator = 6,
        kFrame = 7
    };

    struct SpecialsView {
        uint32_t effectID = kNone;
        std::span<const uint32_t>  integers;
        std::span<const std::string> strings;
        std::span<const float>      floats;
        std::span<const uint8_t>    bools;
    };
}
