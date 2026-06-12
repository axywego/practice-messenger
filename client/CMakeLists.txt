add_executable(client main.cpp)

target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(client PRIVATE
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
)

target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})