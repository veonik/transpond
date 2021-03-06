cmake_minimum_required(VERSION 3.2)
project(transpond)

include(CMakeListsPrivate.txt)

add_definitions(-DF_CPU=16000000L)
add_definitions(-DPLATFORMIO=021003)
add_definitions(-DARDUINO_ARCH_AVR)
add_definitions(-DARDUINO_AVR_UNO)
add_definitions(-DARDUINO=10608)
add_definitions(-D__AVR_ATmega328P__)
add_definitions(-D__AVR__)

add_custom_target(
    PLATFORMIO_BUILD ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPLOAD ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target upload
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_CLEAN ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_PROGRAM ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target program
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPLOADFS ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target uploadfs
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPDATE_ALL ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion update
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(transpond
        src/handset/controller/dashboard.cpp
        src/transponder/fram.cpp
        src/handset/controller/graphics_test.cpp
        src/handset/gui.cpp
        src/handset/handset.cpp
        src/handset/controller/input.cpp
        src/common/message.cpp
        src/handset/controller/position.cpp
        src/handset/controller/programs.cpp
        src/handset/controller/settings.cpp
        src/transponder/transponder.cpp
        src/handset/controller/log_config.cpp
        )
