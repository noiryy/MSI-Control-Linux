cmake_minimum_required(VERSION 3.22)

project(msi_ctl_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
)

set(HEADERS
    src/mainwindow.h
)

set(RESOURCES
    resources.qrc
)

qt_add_executable(msi-ctl-gui
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(msi-ctl-gui PRIVATE Qt6::Widgets)

install(TARGETS msi-ctl-gui RUNTIME DESTINATION bin)
install(FILES assets/msi-ctl.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES msi-ctl.desktop DESTINATION share/applications)
