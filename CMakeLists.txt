cmake_minimum_required(VERSION 3.16)

project(ATM_Simulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Quick QuickControls2 Charts PrintSupport REQUIRED)

set(SOURCE_FILES
    src/main.cpp
    src/AppController.cpp
    src/models/AccountModel.cpp
    src/models/TransactionModel.cpp
    src/viewmodels/AccountViewModel.cpp
    src/viewmodels/TransactionViewModel.cpp
)

set(HEADER_FILES
    src/AppController.h
    src/models/AccountModel.h
    src/models/TransactionModel.h
    src/viewmodels/AccountViewModel.h
    src/viewmodels/TransactionViewModel.h
)

set(RESOURCE_FILES
    resources/resources.qrc
)

qt_add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
    Qt6::PrintSupport
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Installation settings
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Set installation rules for required Qt libraries
if(WIN32)
    qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
    )
    install(SCRIPT ${deploy_script})
endif() 