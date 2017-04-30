cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

file(GLOB LYNX_LWM2M_SRC src/*.c)

include_directories(include)

add_library(paho-mqtt3a STATIC IMPORTED)
add_library(crypto STATIC IMPORTED)
add_library(ssl STATIC IMPORTED)

set_target_properties(paho-mqtt3a PROPERTIES IMPORTED_LOCATION lib/x64/libpaho-mqtt3a.a)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION lib/x64/libcrypto.a)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION lib/x64/libssl.a)


add_library(lynx STATIC ${LYNX_LWM2M_SRC})
add_executable(example example.c)
add_executable(start_test_client test/test.c)

target_link_libraries(
        example
        ${PROJECT_SOURCE_DIR}/bin/lib/x64/liblynx.a
        pthread
        dl
)

target_link_libraries(
        start_test_client
        ${PROJECT_SOURCE_DIR}/bin/lib/x64/liblynx.a
        pthread
        dl
)