add_executable(server main.cpp)

target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(server PRIVATE
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
)

target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})