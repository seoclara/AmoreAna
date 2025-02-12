#----------------------------------------------------------------------------
# Project Name: AMoREI_Anlys (a.k.a. AMoREI analysis script)
# Author: BaseHardware
# Description: A ROOT analysis code and its run script for AMoRE-I
#              simulation with the Geant4
#----------------------------------------------------------------------------
# Setup the project and require CMake version above 3.0
#----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(AMoREI_MCAnlys)

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
    set(MCObjs_LIBPATH $ENV{MCOBJS_PATH}/libMCObjs.dylib)
    set(MCObjs_INCLDIR $ENV{MCOBJS_PATH})
else()
    message(SEND_ERROR "This project cannot be built without MCObjs library. Please check about this.")
endif()

if(DEFINED MCAnlysInfo)
    set(MCAnlysInfo_LIBPATH MCAnlysInfo)
    set(MCAnlysInfo_INCLDIR ${MCAnlysInfo_INCLUDE_DIR})
elseif(DEFINED ENV{MCANLYSINFO_PATH})
    set(MCAnlysInfo_LIBPATH $ENV{MCANLYSINFO_PATH}/libMCAnlysInfo.dylib)
    set(MCAnlysInfo_INCLDIR $ENV{MCANLYSINFO_PATH})
else()
    message(SEND_ERROR "This project cannot be built without MCAnlysInfo library. Please check about this.")
endif()


#----------------------------------------------------------------------------
# Add variables for run scripts
#----------------------------------------------------------------------------
SET(NUM_PER_TASK "100" CACHE STRING "Number of files to process per each task")
SET(IAnlys_TARGET_JOB_NAME "scratch" CACHE STRING "Sample name for analysis")

#----------------------------------------------------------------------------
# Setup global include directories for this project
#----------------------------------------------------------------------------
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Copy all scripts and drawers to the build directory and configure it with setting in CMake
#----------------------------------------------------------------------------

set(RUN_SCRIPTS
    run_MuonAnlys.sh
    run_NeutAnlys.sh
		run_InternalAnlys.sh
		olafjob_submit.sh
    )

foreach(_SCRIPT ${RUN_SCRIPTS})
    configure_file (
        ${PROJECT_SOURCE_DIR}/run/${_SCRIPT}.in
        ${PROJECT_BINARY_DIR}/run/${_SCRIPT}
        )
endforeach()

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include)
file(COPY ${PROJECT_SOURCE_DIR}/include/defs DESTINATION ${PROJECT_BINARY_DIR}/include
    PATTERN *.in EXCLUDE)

#----------------------------------------------------------------------------
# Locate sources and headers for sources of analysis and classification code
#----------------------------------------------------------------------------
file(GLOB MuonAnlys_exec ${PROJECT_SOURCE_DIR}/src/MuonAnlys.cxx)
file(GLOB NeutAnlys_exec ${PROJECT_SOURCE_DIR}/src/NeutAnlys.cxx)
file(GLOB Internal_exec ${PROJECT_SOURCE_DIR}/src/InternalAnlys.cxx)

#----------------------------------------------------------------------------
# Add the executable and link it to the ROOT and basic include directories
#----------------------------------------------------------------------------
add_executable(MuonAnlys_I ${MuonAnlys_exec})
add_executable(NeutAnlys_I ${NeutAnlys_exec})
add_executable(Internal_I ${Internal_exec})
target_include_directories(MuonAnlys_I PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(NeutAnlys_I PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Internal_I PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(MuonAnlys_I ${ROOT_LIBRARIES})
target_link_libraries(NeutAnlys_I ${ROOT_LIBRARIES})
target_link_libraries(Internal_I ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Setup include directories for dependencies (MCObjs)
#----------------------------------------------------------------------------
target_link_libraries(MuonAnlys_I MCAnlysInfo ${MCObjs_LIBPATH})
target_link_libraries(NeutAnlys_I MCAnlysInfo ${MCObjs_LIBPATH})
target_link_libraries(Internal_I MCAnlysInfo ${MCObjs_LIBPATH})
target_include_directories(MuonAnlys_I PUBLIC ${MCObjs_INCLDIR})
target_include_directories(NeutAnlys_I PUBLIC ${MCObjs_INCLDIR})
target_include_directories(Internal_I PUBLIC ${MCObjs_INCLDIR})
include_directories(${MCObjs_INCLDIR})

#----------------------------------------------------------------------------
# Setup include directories for dependencies (MCAnlysInfo)
#----------------------------------------------------------------------------
target_link_libraries(MuonAnlys_I ${MCAnlysInfo_LIBPATH})
target_link_libraries(NeutAnlys_I ${MCAnlysInfo_LIBPATH})
target_link_libraries(Internal_I ${MCAnlysInfo_LIBPATH})
target_include_directories(MuonAnlys_I PUBLIC ${MCAnlysInfo_INCLDIR})
target_include_directories(NeutAnlys_I PUBLIC ${MCAnlysInfo_INCLDIR})
target_include_directories(Internal_I PUBLIC ${MCAnlysInfo_INCLDIR})
include_directories(${MCAnlysInfo_INCLDIR})
