cmake_minimum_required (VERSION 3.5.1)

project (Climber LANGUAGES C CXX)

# On inclut notre bibliotheque dans le processus de CMake
# add_subdirectory(source/routes)

add_definitions(-DTDD)
add_definitions(-D__HEAP_SIZE=150000)
add_definitions(-D_USE_MATH_DEFINES)

if(CMAKE_HOST_UNIX)
set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -Og -rdynamic -fsanitize=undefined -fsanitize=float-divide-by-zero -fno-sanitize-recover")
else()
set(CMAKE_C_FLAGS    "${CMAKE_CXX_FLAGS} -msse2 -mno-ms-bitfields")
add_definitions(-DWIN32)
add_definitions(-D_WIN32_WINNT=0x0600)
add_definitions(-DWINVER=0x600)
include_directories(./TDDW)
endif()


set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -ggdb -fno-omit-frame-pointer --short-enums -MMD -ffunction-sections -fdata-sections -fno-strict-aliasing -fsingle-precision-constant -frounding-math -fsignaling-nans")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x -fno-rtti -fpermissive -fexceptions")


# add the binary tree to the search path for include files

include_directories(
    ./
	TDD
	TDD/drivers
    TDD/nordic
    TDD/logger
	TDD/timer
	TDD/sensors
	TDD/scheduler
	libraries/utils
    libraries/kalman
    libraries/filters
    libraries/pid
    libraries/vnh5019
	rf
    source
	source/model
	source/sensors
	zpm
	pca10040/s332/config
    )

# Cree des variables avec les fichiers a compiler
set(SRCS
    TDD/main_tdd.cpp
    segger_wrapper.c
    TDD/scheduler/task_scheduler.c
    TDD/unit_testing.cpp
    TDD/Simulator.cpp
	TDD/millis_tdd.c
	TDD/timer/timer.c
	TDD/nordic/platform.c
	TDD/nordic/app_scheduler.c
	TDD/logger/tdd_logger.c
	TDD/sensors/vnh5019_driver.c
	TDD/sensors/vl53l1_wrapper.c
	TDD/sensors/lsm6ds33_wrapper.c
	TDD/drivers/ant_tdd.c
	TDD/drivers/gpio_tdd.c
	libraries/kalman/kalman_ext.cpp
	libraries/kalman/UDMatrix.cpp
    libraries/kalman/sine_fitter.cpp
    libraries/pid/arm_pid.c
    libraries/utils/WString.cpp
	libraries/utils/utils.c
	source/Model.cpp
    source/sensors/fram.c
	source/model/UserSettings.cpp
	source/model/data_dispatcher.cpp
	zpm/zpm_decoder.cpp
    )
    
set(HEADERS
    TDD/main_tdd.h
    TDD/Simulator.h
    )
 
# add the executable
add_executable(Climber ${SRCS} ${HEADERS})


if(CMAKE_HOST_UNIX)

target_link_libraries(Climber ubsan)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND size --format=berkeley "${PROJECT_NAME}"
    COMMENT "Invoking: Print Size")

else()

target_link_libraries(Climber imagehlp)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND size --format=berkeley "${PROJECT_NAME}.exe"
    COMMENT "Invoking: Print Size")
endif()

add_custom_target(debug
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
	)
add_dependencies(debug Climber)

add_custom_target(release
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Switch CMAKE_BUILD_TYPE to Release"
	)
add_dependencies(release Climber)
