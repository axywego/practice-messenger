add_executable(testing main.cpp)

target_include_directories(testing PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(testing PRIVATE
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
    SQLiteCpp
)

target_include_directories(testing PRIVATE ${Boost_INCLUDE_DIRS})