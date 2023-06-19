# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -O3 -g3)
project ("BotImpl")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NAME_DEFINITIONS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NAME_DEFINITIONS}")
set(USE_TLS TRUE)
set(USE_OPEN_SSL TRUE)

add_subdirectory(../../lib/TriangleManipulator ./build/TriangleManipulator EXCLUDE_FROM_ALL)
add_subdirectory(../../lib/spdlog ./build/spdlog EXCLUDE_FROM_ALL)

if(NOT DEFINED CHARACTER_NAME_STRING)
  set(CHARACTER_NAME_STRING "DEFAULT")
endif()

add_library(spdlog_HEADERS ALIAS spdlog_header_only)

# Add source to this project's executable.
get_filename_component(prjName "${CMAKE_SOURCE_DIR}_${CHARACTER_NAME_STRING}" NAME_WLE)

add_library(${prjName}_HEADERS INTERFACE)
target_include_directories(${prjName}_HEADERS INTERFACE . ../../include ../../ ../../lib/uvw/src ../../lib/libuv/include ../../lib/TriangleManipulator/include)

add_library ("${prjName}" SHARED
  "src/BotImpl.cpp"
)

target_compile_definitions("${prjName}" PUBLIC "CHARACTER_NAME=${CHARACTER_NAME}" "CHARACTER_CLASS=0${CHARACTER_CLASS}")
# TODO: Add tests and install targets if needed.
target_link_directories(${prjName} PUBLIC ../../ ../../lib/IXWebSocket ../../lib/libuv)
target_link_libraries(${prjName} PUBLIC ${prjName}_HEADERS spdlog_HEADERS Bot ixwebsocket uv TriangleManipulator_HEADERS)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET "${prjName}" PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET "${prjName}" PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries("${prjName}" PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
endif()
set_property(TARGET "${prjName}" PROPERTY COMPILE_OPTIONS "-fPIC")