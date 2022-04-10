cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  AutoSolve
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

CPMAddPackage(
  NAME qamqp
  GIT_TAG v0.6.2
  GITHUB_REPOSITORY frysee/qamqp
  OPTIONS "WITH_TUTORIALS OFF" # create an installable target
)

# ---- Extra non-CPM dependencies ----

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("Looking for Vulkan...")
find_package(Vulkan REQUIRED)
if(${Vulkan_FOUND})
  message("Found Vulkan " ${Vulkan_VERSION})
else()
  message("Couldn't find Vulkan")
endif()

message("Looking for Qt...")
find_package(Qt6 REQUIRED COMPONENTS Network)
if(${Qt6_FOUND})
  message("Found Qt " ${Qt6_VERSION})
else()
  message("Couldn't find Qt6")
endif()

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# ---- Create library ----

qt_add_library(
  ${PROJECT_NAME}
  STATIC
  include/autosolve/exceptions/AutoSolveExceptions.h
  include/autosolve/messaging/AutoSolveBacklogMessage.h
  include/autosolve/messaging/AutoSolveCancelMessage.h
  include/autosolve/messaging/AutoSolveCancelResponse.h
  include/autosolve/messaging/AutoSolveCancelResponseArray.h
  include/autosolve/messaging/AutoSolveConnectionEvent.h
  include/autosolve/messaging/AutoSolveMessage.h
  include/autosolve/messaging/AutoSolveResponse.h
  include/autosolve/messaging/IAutoSolveMessage.h
  include/autosolve/AutoSolve.h
  include/autosolve/AutoSolveGlobal.h
  include/autosolve/AutoSolveConnectResultType.h
  include/autosolve/AutoSolveConstants.h
)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 LINKER_LANGUAGE CXX)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC qamqp::qamqp Qt6::Core Qt6::Network)

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "qamqp 0.6.0"
)
