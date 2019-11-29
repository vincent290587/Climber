cmake_minimum_required (VERSION 3.5.1)

project (Climber)

# On inclut notre bibliotheque dans le processus de CMake
# add_subdirectory(source/routes)

# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

add_definitions(-DTDD)
add_definitions(-D__HEAP_SIZE=150000)
add_definitions(-D_USE_MATH_DEFINES)

set(CMAKE_C_FLAGS  "-fno-builtin --short-enums -nostdlib -MMD -nostdlib -ffunction-sections -fdata-sections -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS  "-g -Og -std=gnu++0x -rdynamic -felide-constructors -fno-exceptions -fno-rtti -fpermissive")


# add the binary tree to the search path for include files

include_directories(
    ./
	TDD
	TDD/drivers
	TDD/timer
	TDD/sensors
	TDD/scheduler
	libraries/utils
    libraries/kalman
    libraries/filters
	rf
    source
	source/model
	source/sensors
    )

# Cree des variables avec les fichiers a compiler
set(SRCS
    TDD/main_tdd.cpp
    segger_wrapper.c
    TDD/scheduler/task_scheduler.c
    TDD/unit_testing.cpp
	TDD/Model_tdd.cpp
    TDD/Simulator.cpp
	TDD/millis_tdd.c
	TDD/order1_filter_tdd.c
	TDD/timer/timer.c
	TDD/drivers/ant_tdd.c
	TDD/drivers/gpio_tdd.cpp
	libraries/kalman/kalman_ext.cpp
	libraries/kalman/UDMatrix.cpp
    libraries/kalman/sine_fitter.cpp
    libraries/utils/WString.cpp
	libraries/utils/utils.c
    source/sensors/fram.c
	source/model/UserSettings.cpp
	source/model/data_dispatcher.cpp
    )
    
set(HEADERS
    TDD/main_tdd.h
    TDD/Simulator.h
    )
 
# add the executable
add_executable(Climber ${SRCS} ${HEADERS})
