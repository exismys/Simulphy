#pragma once

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include "Utilities.hpp"

namespace Simulphy {
void renderImGui(bool canCaptureFrame, bool saveFrameRequested, size_t nVertices, size_t nIndices) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    const ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Frame rate", nullptr,
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoFocusOnAppearing |
                    ImGuiWindowFlags_NoNav);
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Number of Vertices: %s", formatNumberLocale(static_cast<int>(nVertices)).c_str());
    ImGui::Text("Number of Triangles: %s", formatNumberLocale(static_cast<int>(nIndices / 3)).c_str());
    if (!canCaptureFrame) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("save frame")) {
        saveFrameRequested = true;
    }
    if (!canCaptureFrame) {
        ImGui::EndDisabled();
        ImGui::TextDisabled("Frame capture is not supported");
    }
    ImGui::End();
    ImGui::Render();
}
}
