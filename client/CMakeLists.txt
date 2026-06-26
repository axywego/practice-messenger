add_library(client_lib 
    client_handler_qml.hpp
    client_handler_qml.cpp
)

target_include_directories(client_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(client_lib
    PUBLIC 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
)

qt_add_executable(client main.cpp)

target_link_libraries(client PRIVATE
    client_lib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
    ftxui::screen ftxui::dom ftxui::component
)

qt_add_qml_module(client
    URI Client.UI
    VERSION 1.0
    RESOURCE_PREFIX "/"

    QML_FILES
        qml/test.qml
)

if(NOT WIN32)
    include(GNUInstallDirs)
    install(TARGETS client
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()