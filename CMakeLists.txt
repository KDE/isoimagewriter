project(imagewriter)
set(PROJECT_VERSION "0.8")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.10.0")
set(KF5_REQUIRED_VERSION "5.56.0")

find_package(ECM ${KF5_REQUIRED_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)

option(ROSA_BRANDING "Build with ROSA branding" OFF)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

find_package(KF5 ${KF5_REQUIRED_VERSION} REQUIRED COMPONENTS
    I18n
    CoreAddons
    WidgetsAddons
    IconThemes
    Crash
)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(UDev REQUIRED)
  find_package(KF5Auth REQUIRED)
endif()

KDE_ENABLE_EXCEPTIONS()

add_subdirectory(isoimagewriter)
add_subdirectory(images)
add_subdirectory(signing-keys)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
