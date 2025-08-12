cmake_minimum_required(VERSION 3.20)
project(animagic LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
# Try to find all needed ImageMagick components
find_package(ImageMagick REQUIRED COMPONENTS Magick++ MagickCore MagickWand)
# Fallback via pkg-config if components resolution is incomplete
if(NOT ImageMagick_FOUND OR NOT ImageMagick_LIBRARIES)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(IMAGEMAGICK REQUIRED Magick++ MagickCore MagickWand)
  message(STATUS "Using pkg-config for ImageMagick")
  set(ImageMagick_INCLUDE_DIRS ${IMAGEMAGICK_INCLUDE_DIRS})
  set(ImageMagick_LIBRARIES ${IMAGEMAGICK_LIBRARIES})
endif()


add_executable(animagic
    src/main.cpp
    src/resources/resources.qrc

    src/app/Application.cpp
    src/app/Theme.cpp

    src/config/AppSettings.cpp
    src/config/Defaults.cpp

    src/core/MagickBackend.cpp

    src/platform/SpaceCheck.cpp

    src/ui/Main/MainWindow.cpp
    src/ui/Common/TaskBar.cpp
    src/ui/DisassemblePage/DisassemblePage.cpp
    src/ui/DisassemblePage/DisassembleWorker.cpp
    src/ui/AssemblePage/AssemblePage.cpp
    src/ui/AssemblePage/AssembleWorker.cpp
)

target_include_directories(animagic PRIVATE ${ImageMagick_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(animagic PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets ${ImageMagick_LIBRARIES})
add_definitions(-DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
