cmake_minimum_required(VERSION 3.6)
project(FaultTolerantClientServer)

add_executable(client client.c)
target_link_libraries(client setter_increment.c)