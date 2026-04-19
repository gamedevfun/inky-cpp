cmake_minimum_required(VERSION 3.16)
project(inky_cpp VERSION 0.1.0 LANGUAGES CXX)

option(INKY_ENABLE_LINUX_GPIOD "Enable the libgpiod + spidev hardware backend" ON)
option(INKY_REQUIRE_LINUX_GPIOD "Fail configure if INKY_ENABLE_LINUX_GPIOD is ON and libgpiod is unavailable" OFF)
option(INKY_BUILD_EXAMPLES "Build example programs" ON)
option(INKY_BUILD_TESTS "Build test programs" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(inky
    src/board.cpp
    src/e673.cpp
    src/hardware.cpp
    src/palette.cpp
)
add_library(inky::inky ALIAS inky)

target_include_directories(inky
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(inky PUBLIC cxx_std_20)

if(INKY_ENABLE_LINUX_GPIOD)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GPIOD IMPORTED_TARGET libgpiod>=1.6)
    endif()

    if(GPIOD_FOUND)
        target_compile_definitions(inky PUBLIC INKY_ENABLE_LINUX_GPIOD)
        target_link_libraries(inky PRIVATE PkgConfig::GPIOD)
        message(STATUS "Linux gpiod backend enabled")
    else()
        if(INKY_REQUIRE_LINUX_GPIOD)
            message(FATAL_ERROR
                "INKY_REQUIRE_LINUX_GPIOD is ON, but libgpiod>=1.6 was not found.\n"
                "Check that your target sysroot contains libgpiod development files and libgpiod.pc."
            )
        else()
            message(WARNING "libgpiod not found; Linux hardware backend disabled")
        endif()
    endif()
endif()

if(INKY_BUILD_EXAMPLES)
    add_executable(inky_e673_example examples/e673_show.cpp)
    target_link_libraries(inky_e673_example PRIVATE inky)

    add_executable(inky_board_led_example examples/board_led.cpp)
    target_link_libraries(inky_board_led_example PRIVATE inky)

    add_executable(inky_board_buttons_example examples/board_buttons.cpp)
    target_link_libraries(inky_board_buttons_example PRIVATE inky)
endif()

if(INKY_BUILD_TESTS)
    enable_testing()
    add_executable(inky_tests
        tests/test_main.cpp
        tests/test_board.cpp
        tests/test_e673.cpp
        tests/test_palette.cpp
    )
    target_include_directories(inky_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(inky_tests PRIVATE inky)
    add_test(NAME inky_tests COMMAND inky_tests)
endif()

include(GNUInstallDirs)

install(TARGETS inky
    EXPORT inkyTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(INKY_BUILD_EXAMPLES)
    install(TARGETS inky_e673_example
        inky_board_led_example
        inky_board_buttons_example
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
