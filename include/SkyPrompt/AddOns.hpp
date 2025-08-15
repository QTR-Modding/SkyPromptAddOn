#pragma once
#include <algorithm>
#include <numbers>
#include <cmath>
#include <corecrt_math.h>
#include <imgui.h>

namespace SkyPrompt::AddOns {

    namespace SpecialEffects {

        struct SpecialsView {
            uint32_t effectID = 0;
            std::span<const uint32_t>  integers;
            std::span<const std::string> strings;
            std::span<const float>      floats;
            std::span<const uint8_t>    bools;
        };


        namespace {

            void DrawGradientArc(ImDrawList* draw_list, const ImVec2 center, const float radius, const float max_thickness, const ImU32 color,
                                 const float start_angle, const float total_angle, const bool enable_glow, const float min_glow_thickness,
                                 const float glow_thickness_multiplier = 3.0f, const float glow_alpha_multiplier = 0.5f) {

                constexpr int num_segments = 400;

                if (enable_glow) {
                    constexpr int num_glow_layers = 5;

                    for (int layer = 0; layer < num_glow_layers; ++layer) {
                        const float layer_progress = static_cast<float>(layer) / (num_glow_layers - 1);
                        const float current_base_thickness =
                            max_thickness + glow_thickness_multiplier * max_thickness * (1.0f - layer_progress);
                        const float current_alpha_scale = glow_alpha_multiplier * std::pow(layer_progress, 2.f);

                        auto p1_glow = ImVec2(center.x + cosf(start_angle) * radius, center.y + sinf(start_angle) * radius);
                        for (int i = 1; i <= num_segments; ++i) {
                            const float progress = static_cast<float>(i) / num_segments;
                            const float thickness_multiplier = sin(progress * std::numbers::pi_v<float>);
                            const float dynamic_glow_thickness =
                                std::max(current_base_thickness * thickness_multiplier, min_glow_thickness);

                            const float gradient_alpha = sin(progress * std::numbers::pi_v<float>);

                            const ImU32 glow_segment_color =
                                ImGui::GetColorU32(ImVec4((color >> 0 & 0xFF) / 255.0f, (color >> 8 & 0xFF) / 255.0f,
                                                          (color >> 16 & 0xFF) / 255.0f, gradient_alpha * current_alpha_scale));

                            const float angle = start_angle + progress * total_angle;
                            auto p2_glow = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
                            draw_list->AddLine(p1_glow, p2_glow, glow_segment_color, dynamic_glow_thickness);
                            p1_glow = p2_glow;
                        }
                    }
                }

                auto p1_main = ImVec2(center.x + cosf(start_angle) * radius, center.y + sinf(start_angle) * radius);
                for (int i = 1; i <= num_segments; ++i) {
                    const float progress = static_cast<float>(i) / num_segments;
                    const float thickness_multiplier = sin(progress * std::numbers::pi_v<float>);
                    const float dynamic_thickness = max_thickness * thickness_multiplier;

                    const float gradient_alpha = sin(progress * std::numbers::pi_v<float>);

                    const ImU32 segment_color = ImGui::GetColorU32(ImVec4((color >> 0 & 0xFF) / 255.0f, (color >> 8 & 0xFF) / 255.0f,
                                                                          (color >> 16 & 0xFF) / 255.0f, gradient_alpha));

                    const float angle = start_angle + progress * total_angle;
                    auto p2_main = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
                    draw_list->AddLine(p1_main, p2_main, segment_color, dynamic_thickness);
                    p1_main = p2_main;
                }
            }
        }


        inline void VinyArcs(ImDrawList* background_draw_list, const float resScale, const ImVec2 line_center,
                             const float semicircle_radius, const float thickness, const float line_start_angle,
                             const float line_total_arc_angle, const SpecialsView& specials) {

            const auto min_glow_thickness = 0.5f * resScale;
			const auto n_colors = specials.integers.size();
			const auto color1 = n_colors > 0 ? static_cast<ImU32>(specials.integers[0]) : IM_COL32(255, 204, 0, 255);
			const auto color2 = n_colors > 1 ? static_cast<ImU32>(specials.integers[1]) : IM_COL32(200, 160, 0, 255);
			const auto color3 = n_colors > 2 ? static_cast<ImU32>(specials.integers[2]) : IM_COL32(200, 160, 0, 255);

            DrawGradientArc(background_draw_list, line_center, semicircle_radius, thickness, color1,
                            line_start_angle * 0.2f, line_total_arc_angle * 0.2f, true, min_glow_thickness, 6.0f, 0.4f);

            DrawGradientArc(background_draw_list, line_center, semicircle_radius - 20.0f, thickness * 0.4f,
                            color2, line_start_angle * 0.4f, line_total_arc_angle * 0.4f, true, min_glow_thickness, 10.5f,
                            0.8f);

            DrawGradientArc(background_draw_list, line_center, semicircle_radius - 40.0f, thickness,
                            color3, line_start_angle * 0.65f, line_total_arc_angle * 0.65f, true, min_glow_thickness, 4.2f,
                            0.6f);
        }

    }



    inline void RenderSpecialEffect(const SpecialEffects::SpecialsView& a_specials, const ImVec2 a_center, const float a_size, const float resScale) {
        switch (a_specials.effectID) {
            case 1: {
                const float semicircle_radius = a_size * 4 * resScale;
                const float thickness         = 3.0f * resScale;
                constexpr float line_start_angle   = -std::numbers::pi_v<float> / 2.0f;
                constexpr float line_total_arc_angle = std::numbers::pi_v<float>;

                SpecialEffects::VinyArcs(
                    ImGui::GetBackgroundDrawList(),
                    resScale,
                    a_center - ImVec2(semicircle_radius/2.f,0),
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

}
