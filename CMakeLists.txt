cmake_minimum_required(VERSION 3.14)

project(messenger CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(server)
add_subdirectory(client)

add_subdirectory(testing)