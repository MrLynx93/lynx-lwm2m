cmake_minimum_required(VERSION 3.4)
project(lynx_lwm2m)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.c include/lwm2m_attributes.h include/lwm2m_object.h map.cpp map.h src/lwm2m_object.c include/lwm2m_common.h server.h s.h include/lwm2m_access_control.h include/lwm2m_device_management.h tlv_parser.h include/lwm2m_parser.h include/lwm2m_errors.h context.h include/lwm2m.h include/lwm2m_bootstrap.h include/lwm2m_register.h include/lwm2m_information_reporting.h queue_mode.h include/lwm2m_transport.h src/lwm2m_common.c src/lwm2m_access_control.cpp src/lwm2m_access_control.h src/lwm2m_access_control.c src/lwm2m_bootstrap.c src/lwm2m_device_management.c src/lwm2m_transport.c src/lwm2m_parser_serialize.c src/lwm2m_information_reporting.c src/lwm2m_register.c example.c include/lwm2m_map.h include/map.h src/map.c parser_test.c src/lwm2m_parser_deserialize.c src/lwm2m_parser_serialize_discover.c)
add_executable(lynx_lwm2m ${SOURCE_FILES})