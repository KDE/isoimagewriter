project(imagewriter)
set(PROJECT_VERSION "0.1")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.4.0")

find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMPackageConfigHelpers)
include(ECMOptionalAddSubdirectory)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

find_package(KF5 REQUIRED COMPONENTS
    I18n
)

KDE_ENABLE_EXCEPTIONS()

set(imagewriter_SRCS
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

target_link_libraries(imagewriter
    Qt5::Widgets
    KF5::I18n
    dl
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
