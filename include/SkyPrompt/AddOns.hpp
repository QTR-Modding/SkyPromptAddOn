#pragma once
#include "Effects/Types.hpp"
#include "Effects/Separator.hpp"
#include "Effects/Frame.hpp"
#include "Effects/TextBackground.hpp"
#include "Effects/VinyArcs.hpp"
#include <numbers>
#include <imgui.h>

namespace SkyPrompt::AddOns {

    inline void RenderSpecialEffect(const SpecialEffects::SpecialsView& a_specials, const ImVec2 a_center, const float a_size, const float resScale) {
        switch (a_specials.effectID) {
            case SpecialEffects::kVinyArcs: {
                const float semicircle_radius = a_size * 4;
                const float thickness         = 3.0f * resScale;
                constexpr float line_start_angle   = -std::numbers::pi_v<float> / 2.0f;
                constexpr float line_total_arc_angle = std::numbers::pi_v<float>;

                SpecialEffects::VinyArcs(
                    ImGui::GetBackgroundDrawList(),
                    resScale,
                    ImVec2(a_center.x - semicircle_radius / 2.f, a_center.y),
                    semicircle_radius,
                    thickness,
                    line_start_angle,
                    line_total_arc_angle,
                    a_specials
                    );
                break;
            }
            default: break;
        }
    }

    inline void RenderSpecialEffect(const SpecialEffects::SpecialsView& specials, ImDrawList* draw_list,
                                    const ImVec2 text_min, const ImVec2 text_max,
                                    const float angle = 0.0f, const float alpha = 1.0f) {
        if (specials.effectID == SpecialEffects::kTextBackground) {
            SpecialEffects::TextBackground(draw_list, text_min, text_max, angle, alpha, specials);
        }
    }

}
