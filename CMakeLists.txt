cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(imagewriter)
set(PROJECT_VERSION "1.0.0")

set(QT_MIN_VERSION "5.15.2")
set(KF5_REQUIRED_VERSION "5.91.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82.0")

find_package(ECM ${KF5_REQUIRED_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

option(ROSA_BRANDING "Build with ROSA branding" OFF)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Network Widgets DBus Test)

find_package(KF${KF_MAJOR_VERSION} ${KF5_REQUIRED_VERSION} REQUIRED COMPONENTS
    I18n
    CoreAddons
    WidgetsAddons
    IconThemes
    Archive
    Crash
    Solid
)

ecm_set_disabled_deprecation_versions(QT 6.4
     KF 5.101.0
    )

KDE_ENABLE_EXCEPTIONS()

add_subdirectory(isoimagewriter)
add_subdirectory(images)
add_subdirectory(signing-keys)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
