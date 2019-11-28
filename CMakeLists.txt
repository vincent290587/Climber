cmake_minimum_required (VERSION 3.5.1)

project (SpinMeter)

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
    main_tdd.cpp
    segger_wrapper.c
    scheduler/task_scheduler.c
    unit_testing.cpp
	Model_tdd.cpp
    Simulator.cpp
	millis_tdd.c
	order1_filter_tdd.c
	timer/timer.c
	drivers/ant_tdd.c
	drivers/gpio_tdd.cpp
	libraries/kalman/kalman_ext.cpp
	libraries/kalman/UDMatrix.cpp
    libraries/kalman/sine_fitter.cpp
    libraries/utils/WString.cpp
	libraries/utils/utils.c
    source/sensors/fram.c
	source/model/PowerZone.cpp
	source/model/UserSettings.cpp
	source/model/data_dispatcher.cpp
    )
    
set(HEADERS
    main_tdd.h
    Simulator.h
    )
 
# add the executable
add_executable(SpinMeter ${SRCS} ${HEADERS})
