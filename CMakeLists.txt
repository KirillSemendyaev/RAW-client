cmake_minimum_required(VERSION 3.5)
project(Selector)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES udp_server.c raw_client.c)
add_executable(RAW_CLIENT raw_client.c)
add_executable(UDP_SERVER udp_server.c)