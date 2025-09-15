# /*
#  * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
#  * SPDX-License-Identifier: BSD-3-Clause
#  */

set(GPGME_REQUIRED_VERSION "1.8.0")
find_package(Gpgmepp ${GPGME_REQUIRED_VERSION} CONFIG)
find_package(QGpgmeQt6 ${GPGME_REQUIRED_VERSION} CONFIG)

# Set up version information
ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX ISOIMAGEWRITER
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/isoimagewriter_version.h
)

add_executable(isoimagewriter
    usbdevicemonitor.h
    common.cpp
    imagewriter.cpp
    physicaldevice.cpp
    isoverifier.cpp
    fetchisojob.cpp
    isolineedit.cpp
    usbdevicemodel.cpp
    flashcontroller.cpp
    releasefetch.cpp
    filedialogbridge.cpp

    common.h
    mainapplication.h
    imagewriter.h
    physicaldevice.h
    isoverifier.h
    fetchisojob.h
    isolineedit.h
    usbdevicemodel.h
    usbdevice.h
    flashcontroller.h
    releasefetch.h
    filedialogbridge.h

    main.cpp
)

ecm_qt_declare_logging_category(isoimagewriter
    HEADER isoimagewriter_debug.h
    IDENTIFIER ISOIMAGEWRITER_LOG
    CATEGORY_NAME org.kde.isoimagewriter
    DEFAULT_SEVERITY Debug
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    message("Compiling isoimagewriter for Windows")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    target_sources(isoimagewriter PRIVATE
        platform_win.cpp
        usbdevicemonitor_win.cpp
        externalprogressbar_win.cpp
        ../res/isoimagewriter.manifest
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    message("Compiling isoimagewriter for Linux")
    target_sources(isoimagewriter PRIVATE
        platform_lin.cpp
        usbdevicemonitor_lin.cpp
        externalprogressbar_lin.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    message("Compiling isoimagewriter for FreeBSD")
    target_sources(isoimagewriter PRIVATE
        platform_lin.cpp
        usbdevicemonitor_lin.cpp
        externalprogressbar_lin.cpp
    )
else()
    message("Unsupported Platform " . ${CMAKE_SYSTEM_NAME})
endif()

ecm_add_qml_module(isoimagewriter
    GENERATE_PLUGIN_SOURCE
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/isoimagewriter
    URI org.kde.isoimagewriter
)

qt_target_qml_sources(isoimagewriter
    QML_FILES
        qml/Main.qml
        qml/pages/WelcomePage.qml
        qml/pages/IsoListingPage.qml
        qml/pages/DownloadWriteOptionsPage.qml
        qml/pages/DownloadingPage.qml
        qml/pages/FlashPage.qml
        qml/pages/SelectionPage.qml
)


set_target_properties(isoimagewriter PROPERTIES
    OUTPUT_NAME "isoimagewriter"
    VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_compile_definitions(isoimagewriter PRIVATE
    -DPROJECT_VERSION="${PROJECT_VERSION}"
    -DQT_QML_IMPORT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/qml"
)

find_package(KF6I18nLocaleData QUIET)

target_link_libraries(isoimagewriter
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::DBus
        Qt6::Test
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickTemplates2
        KF6::I18n
        KF6::I18nQml
        KF6::CoreAddons
        KF6::WidgetsAddons
        KF6::IconThemes
        KF6::Crash
        KF6::Archive
        KF6::Solid
        KF6::Kirigami
        KF6::IconWidgets
)

if(KF6I18nLocaleData_FOUND)
    target_link_libraries(isoimagewriter PRIVATE KF6::I18nLocaleData)
endif()

target_compile_definitions(isoimagewriter PRIVATE
    -DQT_QML_IMPORT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/qml"
)

# Add GPGME if found
if(QGpgmeQt6_FOUND)
    add_definitions(-D_USE_GPG)
    target_link_libraries(isoimagewriter
        PRIVATE
            QGpgmeQt6
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    target_link_libraries(isoimagewriter
        PRIVATE
            KF6::Solid
            Qt6::DBus
    )

endif()

install(TARGETS isoimagewriter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
