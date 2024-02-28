#----------------------------------------------------------------------------
# Project Name: MCAnlysInfo
# Author: BaseHardware
# Description: Classes for ROOT framework, which is used to store the Hit
#              information, the Track information and the Step information.
#----------------------------------------------------------------------------
# Setup the project and require CMake version above 3.5
#----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(MCAnlysInfo)

#----------------------------------------------------------------------------
# Find ROOT and setup it
#----------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Check dependencies for this project and set include directories and libraries
#----------------------------------------------------------------------------
if(DEFINED MCObjs)
    set(MCObjs_LIBPATH MCObjs)
    set(MCObjs_INCLDIR ${MCObjs_INCLUDE_DIR})
elseif(DEFINED ENV{MCOBJS_PATH})
    set(MCObjs_LIBPATH $ENV{MCOBJS_PATH}/libMCObjs.so)
    set(MCObjs_INCLDIR $ENV{MCOBJS_PATH})
else()
    message(SEND_ERROR "This project cannot be built without MCObjs library. Please check about this.")
endif()

#----------------------------------------------------------------------------
# Setup global include directories for this project
#----------------------------------------------------------------------------
include_directories(${PROJECT_BINARY_DIR}/include)
include_directories(${MCObjs_INCLDIR})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#----------------------------------------------------------------------------
file(GLOB MCAnlysInfo_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB MCAnlysInfo_HEADERS_ORIGINAL ${PROJECT_SOURCE_DIR}/include/*.hxx)
file(GLOB MCAnlysInfo_HEADERS ${PROJECT_BINARY_DIR}/include/*.hxx)

#----------------------------------------------------------------------------
# Copy the header files to build directory
#----------------------------------------------------------------------------
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include)
foreach(NOW_HEADER ${MCAnlysInfo_HEADERS_ORIGINAL})
  configure_file(${NOW_HEADER} ${PROJECT_BINARY_DIR}/include COPYONLY)
endforeach(NOW_HEADER)

#----------------------------------------------------------------------------
# Generate dictionary for classes of this project
#----------------------------------------------------------------------------
ROOT_GENERATE_DICTIONARY(G__MCAnlysInfo
  ${MCAnlysInfo_HEADERS}
  LINKDEF dict/MCAnlysInfo_linkdef.h)

#----------------------------------------------------------------------------
# Add the libraries and link it to the ROOT framework and MCObjs library
#----------------------------------------------------------------------------
add_library(MCAnlysInfo SHARED ${MCAnlysInfo_SOURCES} G__MCAnlysInfo.cxx)
target_link_libraries(MCAnlysInfo ${ROOT_LIBRARIES} ${MCObjs_LIBPATH})
target_include_directories(MCAnlysInfo PUBLIC ${ROOT_INCLUDE_DIRS})
target_include_directories(MCAnlysInfo PUBLIC $ENV{MCOBJS_PATH})

#----------------------------------------------------------------------------
# Expose this public includes to other subprojects through cache variable.
#----------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include
  CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(${PROJECT_NAME}_LIBRARY ${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.so
  CACHE INTERNAL "${PROJECT_NAME}: Path to library" FORCE)
set(${PROJECT_NAME} "Exist!"
  CACHE INTERNAL "${PROJECT_NAME}: Variable for checking dependencies" FORCE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
