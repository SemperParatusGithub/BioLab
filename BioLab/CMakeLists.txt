project(BioLab)

set(HEADERS
    Source/Application.h
    Source/Console.h    
    Source/Core.h
    Source/SerialPort.h

    Source/NodeEditor/Comment.h
    Source/NodeEditor/Node.h
    Source/NodeEditor/NodeEditor.h
    Source/NodeEditor/Scope.h
    Source/NodeEditor/Source.h

    Source/UI/UICore.h
    Source/UI/IconsMaterialDesign.h
    Source/UI/Backends/GLFW_OpenGL.h
    Source/UI/Backends/Win32_DirectX.h

    Source/Util/LiveBuffer.h

    External/stb_image/stb_image.h
)
set(SOURCES
    Source/Main.cpp
    Source/Application.cpp
    Source/Console.cpp
    Source/SerialPort.cpp

    Source/NodeEditor/Comment.cpp
    Source/NodeEditor/Node.cpp
    Source/NodeEditor/NodeEditor.cpp
    Source/NodeEditor/Scope.cpp
    Source/NodeEditor/Source.cpp

    Source/UI/UICore.cpp
    Source/UI/Backends/GLFW_OpenGL.cpp
    Source/UI/Backends/Win32_DirectX.cpp

    Source/Util/LiveBuffer.cpp

    External/stb_image/stb_image.cpp
)

add_executable(BioLab WIN32 ${SOURCES} ${HEADERS})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})


### GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(External/glfw)

### imgui
set(IMGUI_SOURCES
    External/imgui/imgui.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_tables.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/backends/imgui_impl_glfw.h
    External/imgui/backends/imgui_impl_glfw.cpp
    External/imgui/backends/imgui_impl_opengl3.h
    External/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC 
    ${IMGUI_SOURCES}
)
target_include_directories(imgui PRIVATE
    External/imgui
    External/imgui/backends
    External/glfw/include
)
target_link_libraries(imgui PRIVATE
    glfw
    opengl32.lib
)

### implot
set(IMPLOT_SOURCES
    External/implot/implot.h
    External/implot/implot_internal.h
    External/implot/implot.cpp
    External/implot/implot_demo.cpp
    External/implot/implot_items.cpp
)
add_library(implot STATIC
    ${IMPLOT_SOURCES}
)
target_include_directories(implot PRIVATE
    External/implot
    External/imgui
)
target_link_libraries(implot PRIVATE
    imgui
)

### imgui-node-editor
set(IMGUINODEEDITOR_SOURCES
    External/imgui-node-editor/crude_json.h
    External/imgui-node-editor/crude_json.cpp
    External/imgui-node-editor/imgui_bezier_math.h
    External/imgui-node-editor/imgui_bezier_math.inl
    External/imgui-node-editor/imgui_canvas.h
    External/imgui-node-editor/imgui_canvas.cpp
    External/imgui-node-editor/imgui_extra_math.h
    External/imgui-node-editor/imgui_extra_math.inl
    External/imgui-node-editor/imgui_node_editor.h
    External/imgui-node-editor/imgui_node_editor.cpp
    External/imgui-node-editor/imgui_node_editor_api.cpp
    External/imgui-node-editor/imgui_node_editor_internal.h
    External/imgui-node-editor/imgui_node_editor_internal.inl
)
add_library(imgui-node-editor STATIC 
    ${IMGUINODEEDITOR_SOURCES}
)
target_include_directories(imgui-node-editor PRIVATE
    External/imgui-node-editor
    External/imgui
)
target_link_libraries(imgui-node-editor PRIVATE
    imgui
)

target_link_libraries(BioLab
    glfw
    imgui
    implot
    imgui-node-editor
    opengl32.lib

    $<$<PLATFORM_ID:Windows>:d3d12.lib>
    $<$<PLATFORM_ID:Windows>:d3dcompiler.lib>
    $<$<PLATFORM_ID:Windows>:dxgi.lib>
)
target_include_directories(BioLab PUBLIC 
    Source
    External/glfw/include
    External/imgui
    External/implot
    External/imgui-node-editor
    External/imgui/backends
    External/stb_image
)