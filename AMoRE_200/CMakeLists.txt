#----------------------------------------------------------------------------
# Project Name: AMoRE200_Anlys (a.k.a. AMoRE200 analysis script)
# Description: A ROOT analysis code and its run script for AMoRE 2
#              simulation with the Geant4
#----------------------------------------------------------------------------
# Setup the project and require CMake version above 2.8
#----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(AMoRE200_MCAnlys)

#----------------------------------------------------------------------------
# Find ROOT and setup it
#----------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED)
find_package(ROOT COMPONENTS EG)
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

if(DEFINED MCAnlysInfo)
    set(MCAnlysInfo_LIBPATH MCAnlysInfo)
    set(MCAnlysInfo_INCLDIR ${MCAnlysInfo_INCLUDE_DIR})
elseif(DEFINED ENV{MCANLYSINFO_PATH})
    set(MCAnlysInfo_LIBPATH $ENV{MCANLYSINFO_PATH}/libMCAnlysInfo.so)
    set(MCAnlysInfo_INCLDIR $ENV{MCANLYSINFO_PATH})
else()
    message(SEND_ERROR "This project cannot be built without MCAnlysInfo library. Please check about this.")
endif()


#----------------------------------------------------------------------------
# Add variables for run scripts
#----------------------------------------------------------------------------
SET(NUM_PER_TASK "100" CACHE STRING "Number of files to process per each task")
SET(A200Anlys_CLASTag_THRESHOLD "4" CACHE STRING "Threshold for muon tagging classifier")
SET(A200Anlys_TARGET_JOB_NAME "scratch" CACHE STRING "Sample name for analysis")

#----------------------------------------------------------------------------
# Setup global include directories for this project
#----------------------------------------------------------------------------
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Copy all scripts and drawers to the build directory and configure it with setting in CMake
#----------------------------------------------------------------------------

set(RUN_SCRIPTS
		olaf_run.sh
    re_run.sh
    run_MuonAnlys.sh
    run_NeutAnlys.sh
		run_BackgroundAnlys.sh
		run_RockgammaAnlys.sh
		run_CalibrationAnlys.sh
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
file(GLOB CalibrationAnlys_exec ${PROJECT_SOURCE_DIR}/src/CalibrationAnlys.cxx)
file(GLOB BackgroundAnlys_exec ${PROJECT_SOURCE_DIR}/src/BackgroundAnlys.cxx)
file(GLOB RockgammaAnlys_exec ${PROJECT_SOURCE_DIR}/src/RockgammaAnlys.cxx)

#----------------------------------------------------------------------------
# Add the executable and link it to the ROOT and basic include directories
#----------------------------------------------------------------------------
add_executable(MuonAnlys_200   ${MuonAnlys_exec})
add_executable(NeutAnlys_200   ${NeutAnlys_exec})
add_executable(Background_200  ${BackgroundAnlys_exec})
add_executable(Rockgamma_200  ${RockgammaAnlys_exec})
add_executable(Calibration_200 ${CalibrationAnlys_exec})
target_include_directories(MuonAnlys_200   PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(NeutAnlys_200   PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Calibration_200 PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Background_200  PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Rockgamma_200  PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(MuonAnlys_200   ${ROOT_LIBRARIES})
target_link_libraries(NeutAnlys_200   ${ROOT_LIBRARIES})
target_link_libraries(Calibration_200 ${ROOT_LIBRARIES})
target_link_libraries(Background_200  ${ROOT_LIBRARIES})
target_link_libraries(Rockgamma_200  ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Setup include directories for dependencies (MCObjs)
#----------------------------------------------------------------------------
include_directories(${MCObjs_INCLDIR})
target_include_directories(MuonAnlys_200   PUBLIC ${MCObjs_INCLDIR})
target_include_directories(NeutAnlys_200   PUBLIC ${MCObjs_INCLDIR})
target_include_directories(Calibration_200 PUBLIC ${MCObjs_INCLDIR})
target_include_directories(Background_200  PUBLIC ${MCObjs_INCLDIR})
target_include_directories(Rockgamma_200  PUBLIC ${MCObjs_INCLDIR})
target_link_libraries(MuonAnlys_200   ${MCObjs_LIBPATH})
target_link_libraries(NeutAnlys_200   ${MCObjs_LIBPATH})
target_link_libraries(Calibration_200 ${MCObjs_LIBPATH})
target_link_libraries(Background_200  ${MCObjs_LIBPATH})
target_link_libraries(Rockgamma_200  ${MCObjs_LIBPATH})

#----------------------------------------------------------------------------
# Setup include directories for dependencies (MCAnlysInfo)
#----------------------------------------------------------------------------
include_directories(${MCAnlysInfo_INCLDIR})
target_include_directories(MuonAnlys_200   PUBLIC ${MCAnlysInfo_INCLDIR})
target_include_directories(NeutAnlys_200   PUBLIC ${MCAnlysInfo_INCLDIR})
target_include_directories(Calibration_200 PUBLIC ${MCAnlysInfo_INCLDIR})
target_include_directories(Background_200  PUBLIC ${MCAnlysInfo_INCLDIR})
target_include_directories(Rockgamma_200  PUBLIC ${MCAnlysInfo_INCLDIR})
target_link_libraries(MuonAnlys_200 ${MCAnlysInfo_LIBPATH})
target_link_libraries(NeutAnlys_200 ${MCAnlysInfo_LIBPATH})
target_link_libraries(Calibration_200 ${MCAnlysInfo_LIBPATH})
target_link_libraries(Background_200  ${MCAnlysInfo_LIBPATH})
target_link_libraries(Rockgamma_200  ${MCAnlysInfo_LIBPATH})

