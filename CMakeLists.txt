cmake_minimum_required(VERSION 3.16)

project(test_file_xor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Test)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(apptest_file_xor
    main.cpp
    src/AppController.cpp
    src/AppController.h
    src/FileWorker.cpp
    src/FileWorker.h
    src/ProcessingSettings.cpp
    src/ProcessingSettings.h
    src/XorAlgorithm.cpp
    src/XorAlgorithm.h
)

qt_add_qml_module(apptest_file_xor
    URI test_file_xor
    QML_FILES
        Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apptest_file_xor PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest_file_xor
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apptest_file_xor
    PRIVATE Qt6::Quick Qt6::Core
)

enable_testing()

qt_add_executable(app_unit_tests
    tests/app_unit_tests.cpp
    src/AppController.cpp
    src/AppController.h
    src/FileWorker.cpp
    src/FileWorker.h
    src/ProcessingSettings.cpp
    src/ProcessingSettings.h
    src/XorAlgorithm.cpp
    src/XorAlgorithm.h
)

target_include_directories(app_unit_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(app_unit_tests
    PRIVATE Qt6::Test Qt6::Core
)

add_test(NAME app_unit_tests COMMAND app_unit_tests)

if(WIN32)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set_tests_properties(app_unit_tests PROPERTIES
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>;PATH=path_list_prepend:${MINGW_BIN_DIR}"
    )
endif()

install(TARGETS apptest_file_xor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
