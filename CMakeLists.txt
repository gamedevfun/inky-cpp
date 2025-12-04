cmake_minimum_required(VERSION 3.18)
project(inky_cpp LANGUAGES CXX)

option(INKY_ENABLE_LIBGPIOD "Build libgpiod-backed GPIO implementation" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(inky STATIC
    src/e673.cpp
)

if(INKY_ENABLE_LIBGPIOD)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GPIOD REQUIRED libgpiod)
    target_sources(inky PRIVATE
        src/hw/libgpiod_gpio.cpp
        src/hw/spidev_spi.cpp
    )
    target_include_directories(inky PRIVATE ${GPIOD_INCLUDE_DIRS})
    target_link_libraries(inky PRIVATE ${GPIOD_LIBRARIES})
    target_compile_definitions(inky PRIVATE INKY_ENABLE_LIBGPIOD)
else()
    target_sources(inky PRIVATE
        src/hw/spidev_spi.cpp
    )
endif()

target_include_directories(inky
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(inky PUBLIC cxx_std_20)

add_library(inky::inky ALIAS inky)

install(TARGETS inky EXPORT inkyTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include)
install(EXPORT inkyTargets FILE inkyTargets.cmake NAMESPACE inky:: DESTINATION lib/cmake/inky)
