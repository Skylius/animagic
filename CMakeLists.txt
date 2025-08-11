cmake_minimum_required(VERSION 3.20)
project(animagic LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Gui Widgets)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

add_executable(animagic
    src/main.cpp
    src/resources/resources.qrc
    src/app/Theme.cpp
    src/app/Application.cpp
    src/config/AppSettings.cpp
    src/config/Defaults.cpp
    src/platform/SpaceCheck.cpp
    src/platform/Paths.cpp
    src/ui/Main/MainWindow.cpp
    src/ui/Common/TaskBar.cpp
    src/core/TaskRunner.cpp
    src/core/Task.cpp
    src/core/Cancellation.cpp
    src/ui/Common/CenteredListView.cpp
    src/ui/Common/FilePickers.cpp
    src/ui/Common/ProgressLogPane.cpp
    src/ui/SettingsPage/SettingsPage.cpp
    src/ui/MetadataPage/MetadataPage.cpp
    src/ui/AssemblePage/AssemblePage.cpp
    src/ui/AssemblePage/PreviewPlayer.cpp
    src/ui/AssemblePage/FrameTimingEditor.cpp
    src/ui/AssemblePage/FrameListModel.cpp
    src/ui/DisassemblePage/DisassemblePage.cpp
    src/ui/DisassemblePage/DisassembleViewModel.cpp
    src/ui/ProjectBrowser/ProjectBrowser.cpp
    src/ui/ProjectBrowser/Filters.cpp
    src/core/MagickBackend.cpp
    src/core/ProjectMetadata.cpp
    src/core/Profiles.cpp
    src/core/FrameOps.cpp
    src/core/Timing.cpp
    src/core/PreviewEngine.cpp
    src/core/Validators.cpp
    src/ui/AssemblePage/PreviewWidget.cpp
)

target_include_directories(animagic PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(animagic PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent ${ImageMagick_LIBRARIES})

add_definitions(-DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)

target_include_directories(animagic PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)