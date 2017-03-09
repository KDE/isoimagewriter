set(imagewriter_common_SRCS)
ecm_qt_declare_logging_category(imagewriter_common_SRCS
                                HEADER imagewriter_debug.h
                                IDENTIFIER IMAGEWRITER_LOG
                                CATEGORY_NAME org.kde.imagewriter
                                DEFAULT_SEVERITY Debug)

set(imagewriter_SRCS
    ${imagewriter_common_SRCS}
    usbdevicemonitor.h
    common.cpp
    mainapplication.cpp
    maindialog.cpp
    platform_lin.cpp
    externalprogressbar_lin.cpp
    imagewriter.cpp
    physicaldevice.cpp
    platform_lin_suprogram.cpp
    usbdevicemonitor_lin.cpp
    main.cpp
)
ki18n_wrap_ui(imagewriter_SRCS maindialog.ui)

add_executable(imagewriter ${imagewriter_SRCS})

add_executable(testy testy.cpp ${imagewriter_common_SRCS})
target_link_libraries(testy Qt5::Widgets KF5::Auth)

if (ROSA_BRANDING)
    target_compile_definitions(imagewriter PRIVATE -DROSA_BRANDING="1")
endif (ROSA_BRANDING)
target_compile_definitions(imagewriter PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(imagewriter
    Qt5::Widgets
    KF5::I18n
    KF5::CoreAddons
    dl
)

install(TARGETS imagewriter ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.imagewriter.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.imagewriter.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

add_executable(imagewriter_helper imagewriter_helper.cpp ${libkalarm_common_SRCS})
target_link_libraries(imagewriter_helper Qt5::Widgets KF5::Auth KF5::I18n)
install(TARGETS imagewriter_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(imagewriter_helper org.kde.imagewriter root)
kauth_install_actions(org.kde.imagewriter imagewriter.actions)
