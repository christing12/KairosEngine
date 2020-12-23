cmake_minimum_required(VERSION 3.15)

set(IMGUI_PROJ imgui)

project(${IMGUI_PROJ} LANGUAGES CXX)


set(IMGUI_INC
    imconfig.h
    imgui_impl_dx12.h
    imgui_impl_win32.h
    imgui_internal.h
    imgui.h
    imstb_textedit.h
    imstb_truetype.h
)

set(IMGUI_SRC
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_impl_dx12.cpp
    imgui_impl_win32.cpp
    imgui_widgets.cpp
    imgui.cpp
)

add_library(${IMGUI_PROJ} STATIC ${IMGUI_INC} ${IMGUI_SRC})

set_target_properties(${IMGUI_PROJ}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
