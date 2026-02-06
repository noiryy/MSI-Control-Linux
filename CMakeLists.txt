cmake_minimum_required(VERSION 3.22)

project(msi_ctl_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(X11)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/globalshortcut.cpp
    src/mainwindow.cpp
)

set(HEADERS
    src/globalshortcut.h
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
if(X11_FOUND)
    target_compile_definitions(msi-ctl-gui PRIVATE MSI_CTL_HAS_X11)
    target_link_libraries(msi-ctl-gui PRIVATE X11::X11)
endif()

install(TARGETS msi-ctl-gui RUNTIME DESTINATION bin)
install(FILES assets/msi-ctl.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES msi-ctl.desktop DESTINATION share/applications)
