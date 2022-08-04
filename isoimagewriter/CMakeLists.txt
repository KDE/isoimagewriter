set(GPGME_REQUIRED_VERSION "1.8.0")
find_package(Gpgmepp ${GPGME_REQUIRED_VERSION} CONFIG)
find_package(QGpgme ${GPGME_REQUIRED_VERSION} CONFIG)

#find_package(KF5 REQUIRED COMPONENTS QGpgme)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX ISOIMAGEWRITER VERSION_HEADER isoimagewriter_version.h)

set(isoimagewriter_SRCS
    usbdevicemonitor.h
    common.cpp
    mainapplication.cpp
    mainwindow.cpp
    imagewriter.cpp
    physicaldevice.cpp
    main.cpp
    isoverifier.cpp
    fetchisojob.cpp
)

ecm_qt_declare_logging_category(isoimagewriter_SRCS
                                HEADER isoimagewriter_debug.h
                                IDENTIFIER ISOIMAGEWRITER_LOG
                                CATEGORY_NAME org.kde.isoimagewriter
                                DEFAULT_SEVERITY Debug)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    message("Compiling isoimagewriter for Windaes")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(isoimagewriter_SRCS
        ${isoimagewriter_SRCS}
        platform_win.cpp
        usbdevicemonitor_win.cpp
        externalprogressbar_win.cpp
        isoimagewriter.rc
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    message("Compiling isoimagewriter for Linux")
    set(isoimagewriter_SRCS
        ${isoimagewriter_SRCS}
        platform_lin.cpp
        usbdevicemonitor_lin.cpp
        externalprogressbar_lin.cpp
    )
else()
    message("Unsupported Platform " . ${CMAKE_SYSTEM_NAME})
endif()

add_executable(isoimagewriter ${isoimagewriter_SRCS})

if (ROSA_BRANDING)
    target_compile_definitions(isoimagewriter PRIVATE -DROSA_BRANDING="1")
endif (ROSA_BRANDING)
target_compile_definitions(isoimagewriter PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(isoimagewriter
    Qt::Widgets
    Qt::Network
    KF5::I18n
    KF5::CoreAddons
    KF5::WidgetsAddons
    KF5::IconThemes
    KF5::Crash
    KF5::Archive
)
if(QGpgme_FOUND)
add_definitions(-D_USE_GPG)
target_link_libraries(isoimagewriter
    QGpgme
)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_link_libraries(isoimagewriter KF5::Solid)
endif()

if(USE_KAUTH)
  target_link_libraries(isoimagewriter KF5::AuthCore)
endif()

install(TARGETS isoimagewriter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    install(PROGRAMS org.kde.isoimagewriter.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.isoimagewriter.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
endif()

# KAuth
if(USE_KAUTH)
    add_executable(isoimagewriter_helper common.cpp imagewriter_helper.cpp imagewriter.cpp physicaldevice.cpp)
    target_link_libraries(isoimagewriter_helper Qt::Widgets KF5::AuthCore KF5::I18n KF5::WidgetsAddons KF5::Archive)
    install(TARGETS isoimagewriter_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
    kauth_install_helper_files(isoimagewriter_helper org.kde.isoimagewriter root)
    kauth_install_actions(org.kde.isoimagewriter isoimagewriter.actions)
endif()
