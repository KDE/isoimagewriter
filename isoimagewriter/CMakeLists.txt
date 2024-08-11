set(GPGME_REQUIRED_VERSION "1.8.0")
find_package(Gpgmepp ${GPGME_REQUIRED_VERSION} CONFIG)
find_package(QGpgmeQt6 ${GPGME_REQUIRED_VERSION} CONFIG)

#find_package(KF6 REQUIRED COMPONENTS QGpgme)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX ISOIMAGEWRITER VERSION_HEADER isoimagewriter_version.h)

set(isoimagewriter_SRCS
    usbdevicemonitor.h
    common.cpp
    mainapplication.cpp
    mainwindow.cpp
    imagewriter.cpp
    physicaldevice.cpp
    isoverifier.cpp
    fetchisojob.cpp
    isolineedit.cpp

    common.h
    mainapplication.h
    mainwindow.h
    imagewriter.h
    physicaldevice.h
    isoverifier.h
    fetchisojob.h
    isolineedit.h
    main.cpp
)

ecm_qt_declare_logging_category(isoimagewriter_SRCS
                                HEADER isoimagewriter_debug.h
                                IDENTIFIER ISOIMAGEWRITER_LOG
                                CATEGORY_NAME org.kde.isoimagewriter
                                DEFAULT_SEVERITY Debug)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    message("Compiling isoimagewriter for Windows")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(isoimagewriter_SRCS
        ${isoimagewriter_SRCS}
        platform_win.cpp
        usbdevicemonitor_win.cpp
        externalprogressbar_win.cpp
	../res/isoimagewriter.manifest
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    message("Compiling isoimagewriter for Linux")
    set(isoimagewriter_SRCS
        ${isoimagewriter_SRCS}
        platform_lin.cpp
        usbdevicemonitor_lin.cpp
        externalprogressbar_lin.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    message("Compiling isoimagewriter for FreeBSD")
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

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../res/*isoimagewriter.png")
ecm_add_app_icon(isoimagewriter ICONS ${ICONS_SRCS})

target_compile_definitions(isoimagewriter PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(isoimagewriter
    Qt::Widgets
    Qt::Network
    Qt::DBus
    Qt::Test
    KF6::I18n
    KF6::CoreAddons
    KF6::WidgetsAddons
    KF6::IconThemes
    KF6::Crash
    KF6::Archive
    KF6::Solid
)
if(QGpgmeQt6_FOUND)
    add_definitions(-D_USE_GPG)
    target_link_libraries(isoimagewriter QGpgmeQt6)
endif()

target_link_libraries(isoimagewriter KF6::IconWidgets)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_link_libraries(isoimagewriter KF6::Solid)
  target_link_libraries(isoimagewriter Qt::DBus)
endif()

install(TARGETS isoimagewriter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    install(PROGRAMS org.kde.isoimagewriter.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.isoimagewriter.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
endif()
