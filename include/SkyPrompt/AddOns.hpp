#pragma once

namespace SkyPrompt {
    namespace AddOns{

        namespace SpecialEffects {
            namespace {
                ImU32 MulAlpha(const ImU32 c, float a) {
                    a = ImClamp(a, 0.0f, 1.0f);
                    const int A = static_cast<int>((((c >> IM_COL32_A_SHIFT) & 0xFF) * a));
                    return (c & ~IM_COL32_A_MASK) | static_cast<ImU32>(A << IM_COL32_A_SHIFT);
                }

                ImVec2 CenterOf(const ImVec2& pos, const ImVec2& size) {
                    return { pos.x + size.x * 0.5f, pos.y + size.y * 0.5f };
                }

                float Fract(const float x) { return x - floorf(x); }
                
            }
            // your example

            inline void PulseRing(const ImVec2& pos, const ImVec2& size, float alpha) {
                ImDrawList* dl = ImGui::GetForegroundDrawList(ImGui::GetMainViewport());
                const ImVec2 c = CenterOf(pos, size);
                const float  baseR = 0.5f * ImMin(size.x, size.y);

                const double t  = ImGui::GetTime();
                const float  s  = 1.5f;                       // seconds per pulse
                const float  ph = Fract(static_cast<float>(t / s));      // 0..1
                const float  r  = baseR * (0.65f + 0.55f * ph);
                const float  th = ImClamp(r * 0.08f, 1.5f, 6.0f);

                // soft outer glow
                dl->AddCircle(c, r + th * 1.5f, MulAlpha(IM_COL32(255,255,255,40), alpha), 64, th * 0.5f);

                // main ring (fades as it expands)
                const float fade = (1.0f - ph) * (0.6f + 0.4f * sinf(ph * IM_PI));
                dl->AddCircle(c, r, MulAlpha(IM_COL32(255,255,255,180), alpha * fade), 64, th);

                // inner fill ping
                dl->AddCircleFilled(c, r * 0.15f, MulAlpha(IM_COL32(255,255,255,45), alpha * fade), 24);
            }


            
        }

        inline void RenderSpecialEffect(const uint32_t a_effectID, const ImVec2& a_pos, const ImVec2& a_size) {
            switch (a_effectID) {
                case 1:
                    SpecialEffects::PulseRing(a_pos, a_size, 1.0);
                    break;
            }
        }
    }
    
}