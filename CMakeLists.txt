# CMake file for SDL build of SimCoupe

cmake_minimum_required(VERSION 2.6)

project(simcoupe)

include(FindPkgConfig)
include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Release)

set(RESOURCE_DIR ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
add_definitions(-DRESOURCE_DIR="${RESOURCE_DIR}/")

include_directories(Base/ SDL/)

file(GLOB BASE_SRC Base/*.cpp Base/*.c)
file(GLOB SDL_SRC SDL/*.cpp)

# Recommend native Win32/Mac building as they're not well supported yet
if (APPLE)
  message(FATAL_ERROR "Please use Xcode for Mac OS X builds")
elseif (WIN32)
  message(FATAL_ERROR "Please use Visual Studio for Windows builds")
endif ()

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if (COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else ()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif ()

CHECK_CXX_COMPILER_FLAG("-stdlib=libc++" COMPILER_SUPPORTS_STDLIBCXX)
if (COMPILER_SUPPORTS_STDLIBCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

CHECK_CXX_COMPILER_FLAG("-Weffc++" COMPILER_SUPPORTS_EFFCXX)
if (COMPILER_SUPPORTS_EFFCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
endif ()

CHECK_CXX_COMPILER_FLAG("-Wsuggest-override" COMPILER_SUPPORTS_WSUGGEST)
if (COMPILER_SUPPORTS_WSUGGEST)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override")
endif ()

if (CMAKE_BUILD_TOOL MATCHES "make")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
elseif (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif ()


pkg_search_module(SDL2 sdl2)
if (SDL2_FOUND)
  message(STATUS "Using SDL2")
  add_definitions(-DUSE_SDL2)
  include_directories(${SDL2_INCLUDE_DIRS})
  link_directories(${SDL2_LIBRARY_DIRS})
  link_libraries(${SDL2_LIBRARIES})
else ()
  include(FindPkgConfig)
  pkg_search_module(SDL sdl)
  if (SDL_FOUND)
    include_directories(${SDL_INCLUDE_DIRS})
    link_directories(${SDL_LIBRARY_DIRS})
    link_libraries(${SDL_LIBRARIES})
  else ()
    message(SEND_ERROR "This program requires SDL 1.2 or SDL 2.0 [recommended]")
  endif ()
endif ()

find_library(SPECTRUM_LIBRARY NAMES spectrum ENV LD_LIBRARY_PATH)
find_path(SPECTRUM_INCLUDE_DIR libspectrum.h)
if (SPECTRUM_LIBRARY AND SPECTRUM_INCLUDE_DIR)
  message(STATUS "Using libspectrum")
  add_definitions(-DUSE_LIBSPECTRUM)
  include_directories(${SPECTRUM_INCLUDE_DIR})
  link_libraries(${SPECTRUM_LIBRARY})
endif ()

set(CMAKE_THREAD_PREFER_PTHREAD pthread)
find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

find_package(ZLIB)
if (ZLIB_FOUND)
  message(STATUS "Using Zlib")
  add_definitions(-DUSE_ZLIB)
  include_directories(${ZLIB_INCLUDE_DIR})
  link_libraries(${ZLIB_LIBRARY})
endif ()

find_package(BZip2)
if (BZIP2_FOUND)
  message(STATUS "Using BZip2")
  add_definitions(-DUSE_BZIP2)
  include_directories(${BZIP2_INCLUDE_DIR})
  link_libraries(${BZIP2_LIBRARIES})
endif ()

find_library(RESID_LIBRARY NAMES resid ENV LD_LIBRARY_PATH)
find_path(RESID_INCLUDE_DIR resid/sid.h)
if (RESID_LIBRARY AND RESID_INCLUDE_DIR)
  message(STATUS "Using reSID")
  add_definitions(-DUSE_RESID)
  include_directories(${RESID_INCLUDE_DIR})
  link_libraries(${RESID_LIBRARY})
else ()
  message(STATUS "Could NOT find reSID library for SID support")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-D_DEBUG)
endif ()

add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${BASE_SRC} ${SDL_SRC})

install(TARGETS ${PROJECT_NAME}
  DESTINATION bin
)

install(DIRECTORY Resource/
  DESTINATION ${RESOURCE_DIR}
)
