#----------------------------------------------------------------------------
# Project Name: AmoreAna_Release
# Original Author: BaseHardware
# Modified by Jeewon Seo

#----------------------------------------------------------------------------
# Setup the project and require CMake version above 3.5
#----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(AmoreMCAnlys)

#----------------------------------------------------------------------------
# Add variables for the git revision
#----------------------------------------------------------------------------
# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE AmoreMCAnlys_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE AmoreMCAnlys_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

#----------------------------------------------------------------------------
# Add subdirectory contains analysis codes
#----------------------------------------------------------------------------
add_subdirectory(MCAnlysInfo EXCLUDE_FROM_ALL)

#----------------------------------------------------------------------------
# Setup the main include directories
#----------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Check dependencies for this project and set include directories and libraries
#----------------------------------------------------------------------------
if(EXISTS ${PARENT_DIRECTORY}/MCObjs AND DEFINED MCObjs)
    set(MCObjs_LIBPATH MCObjs)
    set(MCObjs_INCLDIR ${MCObjs_INCLUDE_DIR})
elseif(EXISTS ${PROJECT_SOURCE_DIR}/MCObjs)
    add_subdirectory(MCObjs)
    add_dependencies(MCAnlysInfo MCObjs)
elseif(DEFINED ENV{MCOBJS_PATH})
    set(MCObjs_LIBPATH $ENV{MCOBJS_PATH}/libMCObjs.so)
    set(MCObjs_INCLDIR $ENV{MCOBJS_PATH})
else()
    message(SEND_ERROR "This project cannot be built without MCObjs library. Please check about this.")
endif()

#----------------------------------------------------------------------------
# Set cached variables for each packages
#----------------------------------------------------------------------------
set(Enable_MC_AmorePilot OFF CACHE BOOL "Import MC analysis package for AMoRE-Pilot")
set(Enable_MC_Amore200 OFF CACHE BOOL "Import MC analysis package for AMoRE-200")
set(Enable_MC_AmoreI OFF CACHE BOOL "Import MC analysis package for AMoRE-I")

#----------------------------------------------------------------------------
# Add custom targets by settings
#----------------------------------------------------------------------------

if(${Enable_MC_AmorePilot})
    add_subdirectory(AMoRE_Pilot)
    add_dependencies(MuonAnlys_Pilot MCAnlysInfo)
    add_dependencies(NeutAnlys_Pilot MCAnlysInfo)
    add_dependencies(SurfaceAlpha_Pilot MCAnlysInfo)
endif()

if(${Enable_MC_AmoreI})
    add_subdirectory(AMoRE_I)
    add_dependencies(MuonAnlys_I MCAnlysInfo)
    add_dependencies(NeutAnlys_I MCAnlysInfo)
    add_dependencies(Internal_I MCAnlysInfo)
endif()

if(${Enable_MC_Amore200})
    add_subdirectory(AMoRE_200)
    add_dependencies(MuonAnlys_200 MCAnlysInfo)
    add_dependencies(NeutAnlys_200 MCAnlysInfo)
    add_dependencies(Calibration_200 MCAnlysInfo)
    add_dependencies(Background_200 MCAnlysInfo)
    add_dependencies(Rockgamma_200 MCAnlysInfo)
endif()
