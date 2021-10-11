# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -O3 -g)
project ("albot-cpp")
set(USE_TLS TRUE)
set(USE_OPEN_SSL TRUE)

# Add source to this project's executable.
get_filename_component(prjName "${SERVICE_NAME_STRING}" NAME_WLE)

add_library (${prjName} SHARED
  "src/Service.cpp"
)

# TODO: Add tests and install targets if needed.
target_include_directories("${prjName}" PUBLIC . ../../lib/spdlog/include ../../lib/uvw/src ../../lib/libuv/include ../../../src)
target_link_directories(${prjName} PUBLIC ../../ ../../lib/libuv/)
target_link_libraries("${prjName}" PUBLIC Bot uv)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET "${prjName}" PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET "${prjName}" PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries("${prjName}" PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
endif()
set_property(TARGET "${prjName}" PROPERTY COMPILE_OPTIONS "-fPIC")