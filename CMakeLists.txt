cmake_minimum_required(VERSION 3.19)
project(3DViewer VERSION 2.1.2 LANGUAGES C CXX)

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

# ============================================
# Ручной поиск libcgif (для GIF записи)
# ============================================
find_path(CGIF_INCLUDE_DIR
		NAMES cgif.h
		PATHS /usr/include /usr/local/include
)

find_library(CGIF_LIBRARY
		NAMES cgif libcgif
		PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib
)

if(CGIF_INCLUDE_DIR AND CGIF_LIBRARY)
	set(CGIF_FOUND TRUE)
	set(CGIF_INCLUDE_DIRS ${CGIF_INCLUDE_DIR})
	set(CGIF_LIBRARIES ${CGIF_LIBRARY})
	add_compile_definitions(HAS_GIF_SUPPORT)
else()
	set(CGIF_FOUND FALSE)
endif()

# Необходимые файлы и директории
file(GLOB viewer_src_files "viewer/classes/*.cpp")
file(GLOB opengl_src_files "viewer/opengl/*.cpp")
file(GLOB gui_src_files "ui/*.cpp" "include/ui/*.h")
file(GLOB glad_src_files "dependencies/src/glad.c")
file(GLOB gtest_src_files "gtest/*.cpp")
file(GLOB gif_src_files "viewer/services/*.cpp")

# ---------------------------------------------
#     Создание библиотек для работы проекта
# ---------------------------------------------

# Библиотека для записи GIF анимаций
add_library(GIF_LIB STATIC ${gif_src_files})
target_include_directories(GIF_LIB PRIVATE ${CGIF_INCLUDE_DIRS})
target_link_libraries(GIF_LIB ${CGIF_LIBRARIES})

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
	GIF_LIB
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

# Копируем нужные ресурсы в директорию сборки
add_custom_target(CopyShaders ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/viewer/opengl/shaders"
		"${CMAKE_CURRENT_BINARY_DIR}/shaders"
		COMMENT "Updating shader files in build directory..."
)

add_dependencies(3DViewer CopyShaders)

# ---------------------------
# тестирование с помощью GTest
# ----------------------------

# Собираем в режиме отладки
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(GTest REQUIRED)
include(GoogleTest)

enable_testing() # чтобы можно было запускать через ctest

add_executable(TestViewer ${gtest_src_files})

target_compile_definitions(TestViewer PRIVATE
		SHADERS_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/viewer/opengl/shaders/"
		TEST_SHAPES_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/gtest/"
)

target_include_directories(TestViewer PUBLIC)
target_link_libraries(TestViewer
		ViewerClasses
		GTest::gtest_main
)

gtest_discover_tests(TestViewer)
