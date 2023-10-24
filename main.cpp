#include "docking_params.h"
#include "hello_imgui/hello_imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui.h"
#include "m68hc11x.h"
#include "imguiutil.h"
#include "assembler.h"
#include <TextEditor.h>

Assembler assembler;

TextEditor& GetCodeView() {
    static TextEditor codeView;
    return codeView;
}

void WindowAssembler() {
    static TextEditor editor;
    editor.Render("##assembler");

    ImGui::Spacing();
    if (ImGui::RightAlignedButton("Assemble")) {
        std::stringstream ss(editor.GetText());
        assembler.Reset();
        assembler.Assemble(ss);

        std::vector<std::string> finalLines = {};

        for (const Column& col : assembler.lines) {
            char test[200] = {0};

            sprintf(test, "%04x: ", col.offset);

            for (unsigned char i : col.assembled)
                sprintf(test, "%s %02x", test, i);

            sprintf(test, "%s %s", test, col.raw.c_str());

            finalLines.emplace_back(test);
        }

        GetCodeView().SetTextLines(finalLines);
    }
}


void WindowCodeView() {
    TextEditor& codeView = GetCodeView();
    codeView.SetReadOnlyEnabled(true);
    codeView.Render("##codeview");
}

typedef void(*WindowCallback)();
HelloImGui::DockableWindow CreateDockingWindow(const char* label, const char* initialDockSpace, WindowCallback callback, ImGuiWindowFlags flags = ImGuiWindowFlags_None) {
    HelloImGui::DockableWindow window;
    window.label = label;
    window.dockSpaceName = initialDockSpace;
    window.GuiFunction = callback;
    window.imGuiWindowFlags = flags;
    return window;
}

HelloImGui::DockingSplit CreateDockingSplit(const char* initialDock, const char* newDock, ImGuiDir_ direction, f32 ratio) {
    HelloImGui::DockingSplit split;
    split.initialDock = initialDock;
    split.newDock = newDock;
    split.direction = direction;
    split.ratio = ratio;
    return split;
}

HelloImGui::DockingParams CreateDefaultLayout() {
    HelloImGui::DockingParams params;

    params.dockableWindows = {
            CreateDockingWindow("Code View", "LeftSpace", WindowCodeView),
            CreateDockingWindow("Assembler", "RightSpace", WindowAssembler, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar),
    };

    params.dockingSplits = {
            CreateDockingSplit("MainDockSpace", "LeftSpace",  ImGuiDir_Left, 0.5f),
            CreateDockingSplit("MainDockSpace", "RightSpace", ImGuiDir_Right, 0.5f),
    };

    return params;
}

int main(i32, char**) {
    HelloImGui::RunnerParams params;

    params.appWindowParams.windowTitle = m68hc11x::WindowTitle;

    // NOTE(alex): creates MainDockSpace
    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
    // NOTE(alex): allows docking into separate windows entirely
    params.imGuiWindowParams.enableViewports = true;

    params.dockingParams = CreateDefaultLayout();

    HelloImGui::Run(params);

    return 0;
}