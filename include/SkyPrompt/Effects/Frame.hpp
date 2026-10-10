#pragma once
#include "Drawing.hpp"

namespace SkyPrompt::AddOns::SpecialEffects {

    struct Frame {
        enum FloatIndex : uint32_t { kPaddingX, kPaddingY, kRounding, kThickness, kAnimationSpeed };
        static constexpr float defaultPaddingX = 8.0f;
        static constexpr float defaultPaddingY = 8.0f;
        static constexpr float defaultRounding = 4.0f;
        static constexpr float defaultThickness = 2.0f;
        static constexpr ImU32 defaultColor = IM_COL32(255, 255, 255, 128);
        static constexpr float minPadding = -500.0f;
        static constexpr float maxPadding = 500.0f;
        static constexpr float maxRounding = 500.0f;
        static constexpr float maxThickness = 100.0f;

        ImVec2 padding;
        float rounding;
        float thickness;
        float animationSpeed;
        std::span<const uint32_t> colors;

        explicit Frame(const SpecialsView& specials) :
            padding{ReadFloat(specials, kPaddingX, defaultPaddingX, minPadding, maxPadding),
                    ReadFloat(specials, kPaddingY, defaultPaddingY, minPadding, maxPadding)},
            rounding(ReadFloat(specials, kRounding, defaultRounding, 0.0f, maxRounding)),
            thickness(ReadFloat(specials, kThickness, defaultThickness, 0.0f, maxThickness)),
            animationSpeed(ReadFloat(specials, kAnimationSpeed, defaultGradientSpeed, -maxGradientSpeed, maxGradientSpeed)),
            colors(specials.integers.first(std::min(specials.integers.size(), maxGradientColors))) {}

        void Draw(ImDrawList* draw_list, const ImVec2 minimum, const ImVec2 maximum, const float alpha) const {
            if (thickness <= 0.0f || !HasVisibleColor(colors, defaultColor) ||
                maximum.x <= minimum.x || maximum.y <= minimum.y) return;
            const ImVec2 frame_min{minimum.x - padding.x, minimum.y - padding.y};
            const ImVec2 frame_max{maximum.x + padding.x, maximum.y + padding.y};
            if (frame_max.x <= frame_min.x || frame_max.y <= frame_min.y) return;
            const auto faded_color = FadeColor(colors.size() > 1 ? IM_COL32_WHITE :
                colors.empty() ? defaultColor : colors.front(), alpha);
            if ((faded_color & IM_COL32_A_MASK) == 0) return;
            const int first_vertex = draw_list->VtxBuffer.Size;
            const int first_index = draw_list->IdxBuffer.Size;
            draw_list->AddRect(frame_min, frame_max, faded_color, rounding, 0, thickness);
            ApplyGradient(draw_list, first_vertex, first_index, frame_min, {frame_max.x, frame_min.y}, colors, animationSpeed);
        }
    };
}
