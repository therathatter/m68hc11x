cmake_minimum_required(VERSION 3.25)
project(m68hc11)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
        # Enter the desired git tag below
        # GIT_TAG 0.8.0
)

FetchContent_MakeAvailable(hello_imgui)
# Make cmake function `hello_imgui_add_app` available
list(APPEND CMAKE_MODULE_PATH ${HELLOIMGUI_CMAKE_PATH})
include(hello_imgui_add_app)

file(GLOB EDIT_SRC vendor/ImGuiColorTextEdit/*.cpp)

set(BOOST_REGEX_STANDALONE ON)
add_subdirectory(vendor/ImGuiColorTextEdit/vendor/regex)

hello_imgui_add_app(m68hc11 main.cpp imguiutil.cpp assembler.cpp ${EDIT_SRC})

# bad fix for undefined reference in text editor submodule
if(MSVC)
    add_definitions(/FI"math.h")
else()
    # GCC or Clang
    add_definitions(-include math.h)
endif()

target_link_libraries(m68hc11 PRIVATE boost_regex)
target_include_directories(m68hc11 PRIVATE vendor/ImGuiColorTextEdit)

