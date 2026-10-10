#pragma once
#include <algorithm>
#include <array>
#include <numbers>
#include <cmath>
#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <imgui.h>

namespace SkyPrompt::AddOns {

    namespace SpecialEffects {

        inline constexpr size_t maxGradientColors = 3;

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


        namespace {

            inline bool HasVisibleColor(const std::span<const uint32_t> colors, const ImU32 fallback) {
                if (colors.empty()) return (fallback & IM_COL32_A_MASK) != 0;
                return std::any_of(colors.begin(), colors.end(), [](const auto color) { return (color & IM_COL32_A_MASK) != 0; });
            }

            inline void SplitGradientMesh(ImDrawList* draw_list, const int first_vertex, const int first_index,
                                          const ImVec2 start, const ImVec2 direction, const float length_squared) {
                constexpr size_t triangleVertices = 3;
                constexpr size_t maximumSplitVertices = 9;
                constexpr float middleStop = 0.5f;
                const int index_end = draw_list->IdxBuffer.Size;
                const int vertex_count = draw_list->VtxBuffer.Size - first_vertex;
                const auto vertex_offset = draw_list->CmdBuffer.back().VtxOffset;
                std::vector<ImDrawVert> mesh;
                mesh.reserve((index_end - first_index) / triangleVertices * maximumSplitVertices);
                for (int i = first_index; i < index_end; i += triangleVertices) {
                    std::array<ImDrawVert, triangleVertices> triangle;
                    std::array<float, triangleVertices> positions;
                    for (size_t j = 0; j < triangleVertices; ++j) {
                        triangle[j] = draw_list->VtxBuffer[vertex_offset + draw_list->IdxBuffer[i + j]];
                        positions[j] = ((triangle[j].pos.x - start.x) * direction.x +
                            (triangle[j].pos.y - start.y) * direction.y) / length_squared;
                    }
                    for (const bool lower : {true, false}) {
                        std::array<ImDrawVert, triangleVertices + 1> polygon;
                        size_t count = 0;
                        for (size_t j = 0; j < triangleVertices; ++j) {
                            const auto next = (j + 1) % triangleVertices;
                            const bool inside = lower ? positions[j] <= middleStop : positions[j] >= middleStop;
                            const bool next_inside = lower ? positions[next] <= middleStop : positions[next] >= middleStop;
                            if (inside) polygon[count++] = triangle[j];
                            if (inside == next_inside) continue;
                            const float blend = (middleStop - positions[j]) / (positions[next] - positions[j]);
                            const auto& first = triangle[j];
                            const auto& second = triangle[next];
                            auto color = ImGui::ColorConvertU32ToFloat4(first.col);
                            color.w += (ImGui::ColorConvertU32ToFloat4(second.col).w - color.w) * blend;
                            polygon[count++] = {{first.pos.x + (second.pos.x - first.pos.x) * blend,
                                first.pos.y + (second.pos.y - first.pos.y) * blend},
                                {first.uv.x + (second.uv.x - first.uv.x) * blend,
                                first.uv.y + (second.uv.y - first.uv.y) * blend}, ImGui::ColorConvertFloat4ToU32(color)};
                        }
                        for (size_t j = 1; j + 1 < count; ++j) {
                            mesh.insert(mesh.end(), {polygon[0], polygon[j], polygon[j + 1]});
                        }
                    }
                }
                draw_list->PrimUnreserve(index_end - first_index, vertex_count);
                draw_list->_VtxCurrentIdx -= vertex_count;
                draw_list->PrimReserve(static_cast<int>(mesh.size()), static_cast<int>(mesh.size()));
                for (const auto& vertex : mesh) draw_list->PrimVtx(vertex.pos, vertex.uv, vertex.col);
            }

            inline void ApplyGradient(ImDrawList* draw_list, const int first_vertex, const int first_index, const ImVec2 start,
                                      const ImVec2 end, const std::span<const uint32_t> colors) {
                if (colors.size() < 2) return;
                const ImVec2 direction{end.x - start.x, end.y - start.y};
                const float length_squared = direction.x * direction.x + direction.y * direction.y;
                if (length_squared <= 0.0f) return;
                if (colors.size() == maxGradientColors) {
                    SplitGradientMesh(draw_list, first_vertex, first_index, start, direction, length_squared);
                }
                constexpr float maximumAlpha = IM_COL32_A_MASK >> IM_COL32_A_SHIFT;
                for (int i = first_vertex; i < draw_list->VtxBuffer.Size; ++i) {
                    auto& vertex = draw_list->VtxBuffer[i];
                    const float position = std::clamp(((vertex.pos.x - start.x) * direction.x +
                        (vertex.pos.y - start.y) * direction.y) / length_squared, 0.0f, 1.0f) * (colors.size() - 1);
                    const size_t index = std::min(static_cast<size_t>(position), colors.size() - 2);
                    const float blend = position - index;
                    const auto first = ImGui::ColorConvertU32ToFloat4(colors[index]);
                    const auto second = ImGui::ColorConvertU32ToFloat4(colors[index + 1]);
                    const ImVec4 color{first.x + (second.x - first.x) * blend,
                        first.y + (second.y - first.y) * blend, first.z + (second.z - first.z) * blend,
                        (first.w + (second.w - first.w) * blend) *
                            ((vertex.col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) / maximumAlpha};
                    vertex.col = ImGui::ColorConvertFloat4ToU32(color);
                }
            }

            inline float ReadFloat(const SpecialsView& specials, const uint32_t index, const float fallback,
                                   const float minimum, const float maximum) {
                const float value = index < specials.floats.size() ? specials.floats[index] : fallback;
                return std::isfinite(value) ? std::clamp(value, minimum, maximum) : fallback;
            }

            inline ImU32 FadeColor(const ImU32 color, const float alpha) {
                const auto opacity = static_cast<ImU32>(std::lround(
                    ((color & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) *
                    (std::isfinite(alpha) ? std::clamp(alpha, 0.0f, 1.0f) : 0.0f)));
                return (color & ~IM_COL32_A_MASK) | (opacity << IM_COL32_A_SHIFT);
            }

            void DrawGradientArc(ImDrawList* draw_list, const ImVec2 center, const float radius, const float max_thickness, const ImU32 color,
                                 const float start_angle, const float total_angle, const bool enable_glow, const float min_glow_thickness,
                                 const float glow_thickness_multiplier = 3.0f, const float glow_alpha_multiplier = 0.5f) {

                constexpr int num_segments = 600;

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

        struct Separator {
            enum FloatIndex : uint32_t { kThickness, kSpacing, kExtension, kRounding };
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
            std::span<const uint32_t> colors;

            explicit Separator(const SpecialsView& specials) :
                thickness(ReadFloat(specials, kThickness, defaultThickness, 0.0f, maxThickness)),
                spacing(ReadFloat(specials, kSpacing, defaultSpacing, 0.0f, maxSpacing)),
                extension(ReadFloat(specials, kExtension, defaultExtension, minExtension, maxExtension)),
                rounding(ReadFloat(specials, kRounding, defaultRounding, 0.0f, maxRounding)),
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
                ApplyGradient(draw_list, first_vertex, first_index, line_start, line_end, colors);
            }
        };

        struct Frame {
            enum FloatIndex : uint32_t { kPaddingX, kPaddingY, kRounding, kThickness };
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
            std::span<const uint32_t> colors;

            explicit Frame(const SpecialsView& specials) :
                padding{ReadFloat(specials, kPaddingX, defaultPaddingX, minPadding, maxPadding),
                        ReadFloat(specials, kPaddingY, defaultPaddingY, minPadding, maxPadding)},
                rounding(ReadFloat(specials, kRounding, defaultRounding, 0.0f, maxRounding)),
                thickness(ReadFloat(specials, kThickness, defaultThickness, 0.0f, maxThickness)),
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
                ApplyGradient(draw_list, first_vertex, first_index, frame_min, {frame_max.x, frame_min.y}, colors);
            }
        };


        inline void VinyArcs(ImDrawList* background_draw_list, const float resScale, ImVec2 line_center,
                             const float semicircle_radius, const float thickness, const float line_start_angle,
                             const float line_total_arc_angle, const SpecialsView& specials) {

            const auto min_glow_thickness = 0.5f * resScale;
			const auto n_colors = specials.integers.size();
			const auto color1 = n_colors > 0 ? static_cast<ImU32>(specials.integers[0]) : IM_COL32(255, 204, 0, 255);
			const auto color2 = n_colors > 1 ? static_cast<ImU32>(specials.integers[1]) : IM_COL32(200, 160, 0, 255);
			const auto color3 = n_colors > 2 ? static_cast<ImU32>(specials.integers[2]) : IM_COL32(200, 160, 0, 255);

			const auto n_margins = specials.floats.size();
			const auto margin1 = n_margins > 0 ? specials.floats[0] : 0.0f;
			const auto margin2 = n_margins > 1 ? specials.floats[1] : 0.0f;
            line_center.x += margin1 * resScale;
            line_center.y += margin2 * resScale;

			const auto n_bools = specials.bools.size();
			const auto enable_1 = n_bools > 0 ? specials.bools[0] : true;
			const auto enable_2 = n_bools > 1 ? specials.bools[1] : true;
            const auto enable_3 = n_bools > 2 ? specials.bools[2] : true;


            if (enable_3) {
                DrawGradientArc(background_draw_list, line_center, semicircle_radius, thickness, color1,
                                line_start_angle * 0.2f, line_total_arc_angle * 0.2f, true, min_glow_thickness, 6.0f, 0.4f);
			}
            if (enable_2) {
                DrawGradientArc(background_draw_list, line_center, semicircle_radius - 20.0f * resScale, thickness * 0.4f,
                                color2, line_start_angle * 0.4f, line_total_arc_angle * 0.4f, true, min_glow_thickness, 10.5f,
                                0.8f);
            }
            if (enable_1) {
                DrawGradientArc(background_draw_list, line_center, semicircle_radius - 40.0f * resScale, thickness,
                                color3, line_start_angle * 0.65f, line_total_arc_angle * 0.65f, true, min_glow_thickness, 4.2f,
                                0.6f);
            }
        }

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
