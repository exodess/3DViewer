cmake_minimum_required(VERSION 3.19)
project(3DViewer VERSION 2.0 LANGUAGES C CXX)

# настраиваем стандарт языка для компиляции
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# включение заголовочных файлов
include_directories(include)

# для работы Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# смотрим, чтобы были установлены необходимые пакеты
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# Необходимые файлы и директории
file(GLOB viewer_src_files "viewer/classes/*.cpp")
file(GLOB opengl_src_files "viewer/opengl/*.cpp")
file(GLOB gui_src_files "ui/*.cpp" "include/ui/*.h")
file(GLOB glad_src_files "dependencies/src/glad.c")

# Копируем нужные ресурсы в директорию сборки
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/viewer/opengl/shaders/shader.vert"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/viewer/opengl/shaders/shader.geom"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/viewer/opengl/shaders/shader.frag"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# ---------------------------------------------
#     Создание библиотек для работы проекта
# ---------------------------------------------

# Библиотека для работы с классами бэкенда
add_library(ViewerClasses STATIC ${viewer_src_files})

# Библиотека GLAD для инициализации OpenGL
add_library(GLAD_LIB STATIC ${glad_src_files})
target_link_libraries(GLAD_LIB PRIVATE ${CMAKE_DL_LIBS})
target_include_directories(GLAD_LIB PUBLIC dependencies/include)
set_target_properties(GLAD_LIB PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Библиотека для работы с логикой обработки OpenGL
add_library(ViewerGraphicLib STATIC ${glad_src_files} ${opengl_src_files})
target_link_libraries(ViewerGraphicLib 
	ViewerClasses
	GLAD_LIB
	OpenGL::GL
)

# ---------------------------
#      Сборка приложения     
# ---------------------------

add_executable(3DViewer WIN32 main.cpp ${gui_src_files})

target_link_libraries(3DViewer 
	ViewerGraphicLib
	Qt6::Widgets
	Qt6::OpenGLWidgets
)
