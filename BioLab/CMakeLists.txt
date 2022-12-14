project(BioLab)

set(HEADERS
    Source/Application.h
    Source/Console.h    
    Source/Core.h
    Source/SerialPort.h
    Source/UI/UICore.h
    Source/UI/Window.h
    Source/UI/IconsMaterialDesign.h
)

# Local source files here
set(SOURCES
     Source/Main.cpp
     Source/Application.cpp
     Source/Console.cpp
     Source/SerialPort.cpp
     Source/UI/UICore.cpp
     Source/UI/Window.cpp
)

add_executable(BioLab WIN32 ${SOURCES} ${HEADERS})


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

add_library(imgui STATIC ${IMGUI_SOURCES})
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

add_library(implot STATIC ${IMPLOT_SOURCES})
target_include_directories(implot PRIVATE
    External/implot
    External/imgui
)
target_link_libraries(implot PRIVATE
    imgui
)

target_link_libraries(BioLab
    glfw
    imgui
    implot
    opengl32.lib
)

target_include_directories(BioLab
    PUBLIC 
        External/glfw/include
        External/imgui
        External/implot
        External/imgui/backends
)