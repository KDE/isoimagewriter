cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(imagewriter)
set(PROJECT_VERSION "0.9")

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

option(ROSA_BRANDING "Build with ROSA branding" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    option(USE_KAUTH "Build with KAuth (default on for Linux, off for Windows)" ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows)
    option(USE_KAUTH "Build with KAuth (default on for Linux, off for Windows)" OFF)
endif()

if(USE_KAUTH AND CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(KF5Auth REQUIRED)
    add_definitions(-DUSE_KAUTH=ON)
    message("XX using KAuth")
else()
    message("XX not using KAuth")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Network Widgets)

find_package(KF5 ${KF5_REQUIRED_VERSION} REQUIRED COMPONENTS
    I18n
    CoreAddons
    WidgetsAddons
    IconThemes
    Archive
    Crash
    Solid
)

ecm_set_disabled_deprecation_versions(QT 5.15.2
     KF 5.101.0
    )

KDE_ENABLE_EXCEPTIONS()

add_subdirectory(isoimagewriter)
add_subdirectory(images)
add_subdirectory(signing-keys)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
