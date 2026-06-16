cmake_minimum_required(VERSION 3.20)

project(arkis
    VERSION 1.0.0
    DESCRIPTION "ARKIS - Arac Kiralama Sistemi"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

find_package(Qt6 6.5 QUIET COMPONENTS Widgets Charts Network)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    qt_standard_project_setup()

    set(ARKIS_GUI_SOURCES
        src/gui_main.cpp
        src/main_window.cpp
        src/main_window.h
        src/arac_model.cpp
        src/arac_model.h
        src/musteri_model.cpp
        src/musteri_model.h
        src/kiralama_model.cpp
        src/kiralama_model.h
        src/arac_dialog.cpp
        src/arac_dialog.h
        src/musteri_dialog.cpp
        src/musteri_dialog.h
        src/kiralama_dialog.cpp
        src/kiralama_dialog.h
        src/background_tasks.cpp
        src/background_tasks.h
    )

    add_executable(arkis MACOSX_BUNDLE ${ARKIS_GUI_SOURCES})
    target_link_libraries(arkis PRIVATE Qt6::Widgets Qt6::Charts)
    set_target_properties(arkis PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "ARKIS"
        MACOSX_BUNDLE_GUI_IDENTIFIER "tr.edu.arkis"
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

    add_executable(arkis_cli src/main.cpp)

    add_executable(arkis_server src/arkis_server_main.cpp)
    target_link_libraries(arkis_server PRIVATE Qt6::Core Qt6::Network)

    add_executable(arkis_client src/arkis_client_main.cpp)
    target_link_libraries(arkis_client PRIVATE Qt6::Core Qt6::Network)

    set(ARKIS_TARGETS arkis arkis_cli arkis_server arkis_client)
else()
    message(STATUS "Qt6 bulunamadi; sadece komut satiri hedefi derlenecek.")
    add_executable(arkis src/main.cpp)
    set(ARKIS_TARGETS arkis)
endif()

foreach(target ${ARKIS_TARGETS})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
