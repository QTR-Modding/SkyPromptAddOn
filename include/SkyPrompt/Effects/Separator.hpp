#pragma once
#include "Drawing.hpp"
#include <numbers>

namespace SkyPrompt::AddOns::SpecialEffects {

    struct Separator {
        enum FloatIndex : uint32_t { kThickness, kSpacing, kExtension, kRounding, kAnimationSpeed };
        static constexpr float defaultThickness = 2.0f;
        static constexpr float defaultSpacing = 12.0f;
        static constexpr float defaultExtension = 0.0f;
        static constexpr float defaultRounding = 0.0f;
        static constexpr ImU32 defaultColor = IM_COL32(255, 255, 255, 128);
        static constexpr float maxThickness = 100.0f;
        static constexpr float maxSpacing = 500.0f;
        static constexpr float minExtension = -500.0f;
        static constexpr float maxExtension = 500.0f;
        static constexpr float maxRounding = 500.0f;

        float thickness;
        float spacing;
        float extension;
        float rounding;
        float animationSpeed;
        std::span<const uint32_t> colors;

        explicit Separator(const SpecialsView& specials) :
            thickness(ReadFloat(specials, kThickness, defaultThickness, 0.0f, maxThickness)),
            spacing(ReadFloat(specials, kSpacing, defaultSpacing, 0.0f, maxSpacing)),
            extension(ReadFloat(specials, kExtension, defaultExtension, minExtension, maxExtension)),
            rounding(ReadFloat(specials, kRounding, defaultRounding, 0.0f, maxRounding)),
            animationSpeed(ReadFloat(specials, kAnimationSpeed, defaultGradientSpeed, -maxGradientSpeed, maxGradientSpeed)),
            colors(specials.integers.first(std::min(specials.integers.size(), maxGradientColors))) {}

        bool IsVisible() const { return thickness > 0.0f && HasVisibleColor(colors, defaultColor); }

        void Draw(ImDrawList* draw_list, ImVec2 line_start, ImVec2 line_end, const float alpha) const {
            if (!IsVisible()) return;
            const float x = line_end.x - line_start.x;
            const float y = line_end.y - line_start.y;
            const float length = std::hypot(x, y);
            if (length <= 0.0f || length + 2.0f * extension <= 0.0f) return;
            const auto faded_color = FadeColor(colors.size() > 1 ? IM_COL32_WHITE :
                colors.empty() ? defaultColor : colors.front(), alpha);
            if ((faded_color & IM_COL32_A_MASK) == 0) return;
            const ImVec2 offset{x / length * extension, y / length * extension};
            line_start = {line_start.x - offset.x, line_start.y - offset.y};
            line_end = {line_end.x + offset.x, line_end.y + offset.y};
            const int first_vertex = draw_list->VtxBuffer.Size;
            const int first_index = draw_list->IdxBuffer.Size;
            if (rounding == 0.0f) {
                draw_list->AddLine(line_start, line_end, faded_color, thickness);
            } else {
                const float half_thickness = thickness * 0.5f;
                const ImVec2 minimum{line_start.x, line_start.y - half_thickness};
                const ImVec2 maximum{line_start.x + length + 2.0f * extension, line_start.y + half_thickness};
                const float radius = std::min({rounding, half_thickness, (maximum.x - minimum.x) * 0.5f});
                constexpr float quarter_turn = std::numbers::pi_v<float> * 0.5f;
                draw_list->PathArcTo({maximum.x - radius, maximum.y - radius}, radius, 0.0f, quarter_turn);
                draw_list->PathArcTo({minimum.x + radius, maximum.y - radius}, radius, quarter_turn, quarter_turn * 2.0f);
                draw_list->PathArcTo({minimum.x + radius, minimum.y + radius}, radius, quarter_turn * 2.0f, quarter_turn * 3.0f);
                draw_list->PathArcTo({maximum.x - radius, minimum.y + radius}, radius, quarter_turn * 3.0f, quarter_turn * 4.0f);
                draw_list->PathFillConvex(faded_color);
                for (int i = first_vertex; i < draw_list->VtxBuffer.Size; ++i) {
                    auto& position = draw_list->VtxBuffer[i].pos;
                    const ImVec2 relative{position.x - line_start.x, position.y - line_start.y};
                    position = {line_start.x + (relative.x * x - relative.y * y) / length,
                                line_start.y + (relative.x * y + relative.y * x) / length};
                }
            }
            ApplyGradient(draw_list, first_vertex, first_index, line_start, line_end, colors, animationSpeed);
        }
    };
}
