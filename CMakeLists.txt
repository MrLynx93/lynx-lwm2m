cmake_minimum_required(VERSION 3.5.1)

set(SYSROOT $ENV{NDK}/platforms/android-23/arch-arm)
set(CMAKE_C_COMPILER $ENV{NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc-4.9)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT} -std=c99")

file(GLOB LYNX_LWM2M_SRC src/*.c)

include_directories(include)
link_directories(lib)

add_library(lynxlwm2m SHARED ${LYNX_LWM2M_SRC})
add_executable(testapp main.c)

target_link_libraries(lynxlwm2m paho)
target_link_libraries(testapp paho lynxlwm2m)