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
include(ECMQtDeclareLoggingCategory)

option(ROSA_BRANDING "Build with ROSA branding" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBUDEV REQUIRED libudev)
endif()

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

find_package(KF5 REQUIRED COMPONENTS
    I18n
    CoreAddons
    Auth
    WidgetsAddons
    IconThemes
)

KDE_ENABLE_EXCEPTIONS()

add_subdirectory(isoimagewriter)
add_subdirectory(images)
add_subdirectory(signing-keys)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
