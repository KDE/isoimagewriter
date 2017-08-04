set(GPGME_REQUIRED_VERSION "1.8.0")
find_package(Gpgmepp ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)
find_package(QGpgme ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)

set(isoimagewriter_common_SRCS
    platform_lin.cpp
    common.cpp
)
ecm_qt_declare_logging_category(isoimagewriter_common_SRCS
                                HEADER isoimagewriter_debug.h
                                IDENTIFIER ISOIMAGEWRITER_LOG
                                CATEGORY_NAME org.kde.isoimagewriter
                                DEFAULT_SEVERITY Debug)

set(isoimagewriter_SRCS
    ${isoimagewriter_common_SRCS}
    usbdevicemonitor.h
    common.cpp
    mainapplication.cpp
    maindialog.cpp
    platform_lin.cpp
    externalprogressbar_lin.cpp
    imagewriter.cpp
    physicaldevice.cpp
    usbdevicemonitor_lin.cpp
    main.cpp
    verifyisoworker.cpp
    verifyiso.cpp
    verifyneoniso.cpp
    verifykubuntuiso.cpp
    verifynetrunneriso.cpp
    verifyarchiso.cpp
)
ki18n_wrap_ui(isoimagewriter_SRCS maindialog.ui)

add_executable(isoimagewriter ${isoimagewriter_SRCS})

#add_executable(testy testy.cpp ${isoimagewriter_common_SRCS})
#target_link_libraries(testy Qt5::Widgets KF5::Auth)

if (ROSA_BRANDING)
    target_compile_definitions(isoimagewriter PRIVATE -DROSA_BRANDING="1")
endif (ROSA_BRANDING)
target_compile_definitions(isoimagewriter PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(isoimagewriter
    Qt5::Widgets
    KF5::I18n
    KF5::CoreAddons
    KF5::Auth
    QGpgme
    dl
    KF5::WidgetsAddons
    KF5::IconThemes
)
#target_link_libraries(testy
#    Qt5::Widgets
#    KF5::I18n
#    KF5::CoreAddons
#    dl
#    QGpgme
#    Gpgmepp
#)

install(TARGETS isoimagewriter ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.isoimagewriter.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.isoimagewriter.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

add_executable(isoimagewriter_helper imagewriter_helper.cpp imagewriter.cpp physicaldevice.cpp)
target_link_libraries(isoimagewriter_helper Qt5::Widgets KF5::Auth KF5::I18n KF5::WidgetsAddons)
install(TARGETS isoimagewriter_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(isoimagewriter_helper org.kde.isoimagewriter root)
kauth_install_actions(org.kde.isoimagewriter isoimagewriter.actions)
