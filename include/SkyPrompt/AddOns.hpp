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

            inline void DrawGradientArc(ImDrawList* draw_list, ImVec2 center, float radius, float max_thickness, ImU32 color,
                float start_angle, float total_angle, bool enable_glow = false,
                float glow_thickness_multiplier = 3.0f, float glow_alpha_multiplier = 0.5f) {
                const int num_segments = 400;

                if (enable_glow) {
                    const int num_glow_layers = 5;
                    const float min_glow_thickness = 0.5f * GetResolutionScale();

                    for (int layer = 0; layer < num_glow_layers; ++layer) {
                        float layer_progress = static_cast<float>(layer) / (num_glow_layers - 1);
                        float current_base_thickness =
                            max_thickness + (glow_thickness_multiplier * max_thickness) * (1.0f - layer_progress);
                        float current_alpha_scale = glow_alpha_multiplier * pow(layer_progress, 2);

                        ImVec2 p1_glow = ImVec2(center.x + cosf(start_angle) * radius, center.y + sinf(start_angle) * radius);
                        for (int i = 1; i <= num_segments; ++i) {
                            float progress = static_cast<float>(i) / num_segments;
                            float thickness_multiplier = sin(progress * std::numbers::pi_v<float>);
                            float dynamic_glow_thickness =
                                std::max(current_base_thickness * thickness_multiplier, min_glow_thickness);

                            // --- MUDANÇA AQUI ---
                            // Substituímos o if/else pelo seno para um gradiente perfeitamente suave.
                            float gradient_alpha = sin(progress * std::numbers::pi_v<float>);

                            ImU32 glow_segment_color =
                                ImGui::GetColorU32(ImVec4(((color >> 0) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f,
                                    ((color >> 16) & 0xFF) / 255.0f, gradient_alpha * current_alpha_scale));

                            const float angle = start_angle + progress * total_angle;
                            ImVec2 p2_glow = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
                            draw_list->AddLine(p1_glow, p2_glow, glow_segment_color, dynamic_glow_thickness);
                            p1_glow = p2_glow;
                        }
                    }
                }

                // --- DESENHO DO ARCO PRINCIPAL (COM A MESMA MUDANÇA) ---
                ImVec2 p1_main = ImVec2(center.x + cosf(start_angle) * radius, center.y + sinf(start_angle) * radius);
                for (int i = 1; i <= num_segments; ++i) {
                    float progress = static_cast<float>(i) / num_segments;
                    float thickness_multiplier = sin(progress * std::numbers::pi_v<float>);
                    float dynamic_thickness = max_thickness * thickness_multiplier;

                    // --- MUDANÇA AQUI ---
                    // Usamos a mesma função seno para a transparência do arco principal.
                    float gradient_alpha = sin(progress * std::numbers::pi_v<float>);

                    ImU32 segment_color = ImGui::GetColorU32(ImVec4(((color >> 0) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f,
                        ((color >> 16) & 0xFF) / 255.0f, gradient_alpha));

                    const float angle = start_angle + progress * total_angle;
                    ImVec2 p2_main = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
                    draw_list->AddLine(p1_main, p2_main, segment_color, dynamic_thickness);
                    p1_main = p2_main;
                }
            }
            void ImGui::Renderer::Manager::ShowQueue() {
                if (IsPaused()) {
                    return;
                }
                const auto [width, height] = RE::BSGraphics::Renderer::GetScreenSize();

                // Centro da tela, usado como âncora para a linha amarela decorativa.
                const ImVec2 line_center(width * 0.5f, height * 0.5f);

                // Centro dos BOTÕES, com o deslocamento horizontal que você pode ajustar.
                const float horizontal_offset = 0.0f;  // Altere este valor para mover os botões
                const ImVec2 buttons_center(width * 0.5f + horizontal_offset, height * 0.5f);

                // --- 2. JANELA PRINCIPAL TRANSPARENTE ("SkyPrompt") ---
                // Esta janela serve como uma "tela de pintura" invisível para os nossos elementos.
                ImGui::SetNextWindowPos(ImVec2(0, 0));  // A janela principal pode ocupar a tela toda
                ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

                // Define o estilo para esta janela ser 100% transparente e sem bordas
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

                // Usamos ImGuiWindowFlags_NoInputs para que ela não bloqueie cliques do mouse no jogo.
                ImGui::Begin("SkyPrompt", nullptr,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

                // Pega a lista de desenho DE FUNDO para que o semicírculo apareça atrás dos botões.
                ImDrawList* background_draw_list = ImGui::GetBackgroundDrawList();

                // --- 3. LÓGICA DE DESENHO (Cálculos de Posição dos Botões e Desenho do Arco) ---
                std::map<RefID, std::vector<SubManager*>> object_managers;
                int active_manager_count = 0;
                for (const auto& a_manager : managers) {
                    if (a_manager->HasQueue() && !a_manager->GetAttachedObject()) {
                        active_manager_count++;
                    }
                }

                const float semicircle_radius = 150.0f * GetResolutionScale();
                const float item_offset = 10.0f * GetResolutionScale();  // Aumentei um pouco para dar mais espaço
                const float items_radius = semicircle_radius + item_offset;
                // --- ÂNGULOS PARA A LINHA DECORATIVA (Sempre 180 graus) ---
                const float line_total_arc_angle = std::numbers::pi_v<float>;      // PI = 180 graus
                const float line_start_angle = -std::numbers::pi_v<float> / 2.0f;  // Começa no topo (-90 graus)

                // --- ÂNGULOS PARA OS BOTÕES (Com padding ajustável) ---
                const float angular_padding_degrees = 60.0f;  // para agrupar/separar os botões
                const float angular_padding_rad = angular_padding_degrees * std::numbers::pi_v<float> / 180.0f;
                const float buttons_total_arc_angle = std::numbers::pi_v<float> -(2.0f * angular_padding_rad);
                const float buttons_start_angle = (-std::numbers::pi_v<float> / 2.0f) + angular_padding_rad;

                // O passo do ângulo dos botões usa o ângulo total dos botões
                float angle_step = (active_manager_count > 1) ? (buttons_total_arc_angle / (active_manager_count - 1)) : 0.0f;

                // O ângulo corrente inicial para o loop dos botões
                float current_angle = buttons_start_angle;
                const float thickness = 3.0f * GetResolutionScale(); // grosura da linha
                // --- DESENHO DO ARCO COM GRADIENTE (usando line_center) ---
                if (active_manager_count > 0) {
                    // Arco 1 (o original)
                    DrawGradientArc(background_draw_list, line_center, semicircle_radius, thickness, IM_COL32(255, 204, 0, 255),
                        line_start_angle * 0.2f, line_total_arc_angle * 0.2f, true, 6.0f, 0.4f);

                    // Arco 2 (um pouco menor e mais fino, com outra cor)
                    DrawGradientArc(background_draw_list, line_center, semicircle_radius - 20.0f, thickness * 0.4f,
                        IM_COL32(200, 160, 0, 255), line_start_angle * 0.4f, line_total_arc_angle * 0.4f, true, 10.5f,
                        0.8f);

                    DrawGradientArc(background_draw_list, line_center, semicircle_radius - 40.0f, thickness,
                        IM_COL32(200, 160, 0, 255), line_start_angle * 0.65f, line_total_arc_angle * 0.65f, true, 4.2f,
                        0.6f);


                }


                // --- 5. DESENHO DOS BOTÕES E TEXTOS (usando buttons_center) ---
                int manager_idx = 0;
                for (auto& a_manager : managers) {
                    if (const auto a_ref = a_manager->GetAttachedObject()) {
                        object_managers[a_ref->GetFormID()].push_back(a_manager.get());
                        continue;
                    }

                    if (a_manager->HasQueue()) {
                        // Calcula a posição de cada botão usando o `buttons_center` deslocado
                        ImVec2 promptPos = { buttons_center.x + items_radius * cos(current_angle),
                                            buttons_center.y + items_radius * sin(current_angle) };

                        // Cria uma janela invisível para cada botão nessa posição
                        ImGui::SetNextWindowPos(promptPos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
                        std::string window_name = std::format("PromptWindow_{}", manager_idx++);

                        // Estilo transparente para a janela do botão (já estava correto)
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

                        ImGui::Begin(window_name.c_str(), nullptr,
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav |
                            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs |
                            ImGuiWindowFlags_NoBringToFrontOnFocus);

                        // Chama a função que desenha o ícone e o texto rotacionado
                        a_manager->ShowQueue(current_angle);

                        ImGui::End();
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar(2);

                        //// Desenha uma linha após cada botão, mas não após o último.
                        //if (manager_idx < active_manager_count) {
                        //    // Pega a mesma lista de desenho do fundo usada pelo arco
                        //    ImDrawList* separator_draw_list = ImGui::GetBackgroundDrawList();
                        //    // Define uma espessura menor para as linhas separadoras
                        //    const float separator_thickness = 2.0f * GetResolutionScale();

                        //    // 1. Calcula o ângulo para a linha, que fica no meio do caminho entre este botão e o próximo.
                        //    float separator_angle = current_angle + (angle_step / 2.0f);

                        //    // 2. Calcula a cor do gradiente para esta posição, usando a mesma lógica do arco principal.
                        //    float separator_progress = (separator_angle - start_angle) / total_arc_angle;
                        //    float alpha_multiplier = 1.0f;
                        //    if (separator_progress < 0.5f) {
                        //        alpha_multiplier = separator_progress * 2.0f;
                        //    } else {
                        //        alpha_multiplier = 1.0f - ((separator_progress - 0.5f) * 2.0f);
                        //    }
                        //    alpha_multiplier = std::clamp(alpha_multiplier, 0.0f, 1.0f);
                        //    const ImU32 separator_color = IM_COL32(255, 204, 0, static_cast<int>(255 * alpha_multiplier));

                        //    // 3. Calcula o ponto inicial e final da linha separadora.
                        //    // Ela começa no arco amarelo e vai um pouco além dos botões.
                        //    ImVec2 line_start = {line_center.x + (semicircle_radius - (thickness / 2.0f)) * cosf(separator_angle),
                        //                         line_center.y + (semicircle_radius - (thickness / 2.0f)) * sinf(separator_angle)};
                        //    ImVec2 line_end = {
                        //        line_center.x + (items_radius + 10.0f * GetResolutionScale()) * cosf(separator_angle),
                        //        line_center.y + (items_radius + 10.0f * GetResolutionScale()) * sinf(separator_angle)};

                        //    // 4. Desenha a linha.
                        //    separator_draw_list->AddLine(line_start, line_end, separator_color, separator_thickness);
                        //}

                        current_angle += angle_step;
                    }
                }

                // --- 6. FIM DA JANELA PRINCIPAL E HOVER PROMPTS ---
                ImGui::End();            // Finaliza a janela "SkyPrompt"
                ImGui::PopStyleVar(2);   // Restaura BorderSize e Padding
                ImGui::PopStyleColor();  // Restaura a cor de fundo

                // Loop para prompts em objetos (hover) - também com fundo transparente
                int i = 0;
                for (const auto& managers_ : object_managers | std::views::values) {
                    auto window_pos = managers_[0]->GetAttachedObjectPos();
                    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

                    ImGui::Begin(std::format("SkyPromptHover{}", i++).c_str(), nullptr,
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);
                    for (const auto a_manager : managers_) {
                        a_manager->ShowQueue(0.0f);  // Hover prompts não precisam de rotação
                    }
                    ImGui::End();
                    ImGui::PopStyleColor();
                }
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