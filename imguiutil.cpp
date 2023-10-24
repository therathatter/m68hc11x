
#include "imguiutil.h"
#include "imgui.h"

bool ImGui::RightAlignedButton(const char* text) {
    auto& style = ImGui::GetStyle();
    float width = ImGui::CalcTextSize(text).x + style.FramePadding.x * 2.f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width);
    return ImGui::Button(text);
}