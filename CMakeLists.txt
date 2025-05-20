cmake_minimum_required(VERSION 3.30)
project(C_Web_Server_IO_Multiplexing C)

set(CMAKE_C_STANDARD 23)

add_executable(C_Web_Server_IO_Multiplexing_Select select-server.c)
add_executable(C_Web_Server_IO_Multiplexing_Poll poll-server.c)
