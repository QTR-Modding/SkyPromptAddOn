#pragma once
#include "Types.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include <imgui.h>

namespace SkyPrompt::AddOns::SpecialEffects {

    inline constexpr size_t maxGradientColors = 3;
    inline constexpr float defaultGradientSpeed = 0.0f;
    inline constexpr float maxGradientSpeed = 10.0f;

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

        inline ImVec4 BlendColor(const ImVec4 first, const ImVec4 second, const float blend) {
            return {std::lerp(first.x, second.x, blend), std::lerp(first.y, second.y, blend),
                    std::lerp(first.z, second.z, blend), std::lerp(first.w, second.w, blend)};
        }

        inline void ApplyGradient(ImDrawList* draw_list, const int first_vertex, const int first_index, const ImVec2 start,
                                  const ImVec2 end, const std::span<const uint32_t> colors, const float animation_speed) {
            if (colors.size() < 2) return;
            const ImVec2 direction{end.x - start.x, end.y - start.y};
            const float length_squared = direction.x * direction.x + direction.y * direction.y;
            if (length_squared <= 0.0f) return;
            if (colors.size() == maxGradientColors) {
                SplitGradientMesh(draw_list, first_vertex, first_index, start, direction, length_squared);
            }
            double phase = animation_speed == 0.0f ? 0.0 : std::fmod(ImGui::GetTime() * animation_speed, 1.0);
            if (phase < 0.0) phase += 1.0;
            const float offset = static_cast<float>(phase * colors.size());
            std::array<ImVec4, maxGradientColors> stops;
            for (size_t i = 0; i < colors.size(); ++i) {
                const float position = i + offset;
                const size_t index = static_cast<size_t>(position) % colors.size();
                stops[i] = BlendColor(ImGui::ColorConvertU32ToFloat4(colors[index]),
                    ImGui::ColorConvertU32ToFloat4(colors[(index + 1) % colors.size()]), position - std::floor(position));
            }
            constexpr float maximumAlpha = IM_COL32_A_MASK >> IM_COL32_A_SHIFT;
            for (int i = first_vertex; i < draw_list->VtxBuffer.Size; ++i) {
                auto& vertex = draw_list->VtxBuffer[i];
                const float position = std::clamp(((vertex.pos.x - start.x) * direction.x +
                    (vertex.pos.y - start.y) * direction.y) / length_squared, 0.0f, 1.0f) * (colors.size() - 1);
                const size_t index = std::min(static_cast<size_t>(position), colors.size() - 2);
                const float blend = position - index;
                auto color = BlendColor(stops[index], stops[index + 1], blend);
                color.w *= ((vertex.col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) / maximumAlpha;
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
    }

}
