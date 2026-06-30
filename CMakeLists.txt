cmake_minimum_required(VERSION 3.14)

project(messenger CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql Gui Quick Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql Gui Quick Qml)

qt6_standard_project_setup()

include(FetchContent)
FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG        3.3.3
)
set(SQLITECPP_RUN_CPPLINT   OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPCHECK  OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SQLiteCpp)

add_subdirectory(server)
add_subdirectory(client)

add_subdirectory(testing)