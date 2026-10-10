#pragma once
#include "Types.hpp"
#include <algorithm>
#include <cmath>
#include <imgui.h>

namespace SkyPrompt::AddOns::SpecialEffects {

    inline void TextBackground(ImDrawList* draw_list, const ImVec2 text_min, const ImVec2 text_max,
                               const float angle, const float alpha, const SpecialsView& specials) {
        if (text_max.x <= text_min.x || text_max.y <= text_min.y) return;
        const auto n_floats = specials.floats.size();
        const ImVec2 padding{n_floats > 0 ? specials.floats[0] : 0.0f,
                             n_floats > 1 ? specials.floats[1] : 0.0f};
        const float rounding = n_floats > 2 ? std::max(specials.floats[2], 0.0f) : 0.0f;
        const ImVec2 background_min{text_min.x - padding.x, text_min.y - padding.y};
        const ImVec2 background_max{text_max.x + padding.x, text_max.y + padding.y};
        if (background_max.x <= background_min.x || background_max.y <= background_min.y) return;
        auto color = specials.integers.empty() ? IM_COL32(0, 0, 0, 128) : specials.integers.front();
        const auto opacity = static_cast<ImU32>(std::lround(
            ((color & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) * std::clamp(alpha, 0.0f, 1.0f)));
        color = (color & ~IM_COL32_A_MASK) | (opacity << IM_COL32_A_SHIFT);
        const auto first_vertex = draw_list->VtxBuffer.Size;
        draw_list->AddRectFilled(background_min, background_max, color, rounding);
        if (angle == 0.0f) return;

        const ImVec2 center{(text_min.x + text_max.x) * 0.5f, (text_min.y + text_max.y) * 0.5f};
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        for (auto i = first_vertex; i < draw_list->VtxBuffer.Size; ++i) {
            auto& position = draw_list->VtxBuffer[i].pos;
            const float x = position.x - center.x;
            const float y = position.y - center.y;
            position = {center.x + x * cosine - y * sine, center.y + x * sine + y * cosine};
        }
    }
}
