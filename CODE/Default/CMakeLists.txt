# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)

add_definitions(-std=c++20)

project ("albot-cpp")

# Add source to this project's executable.
get_filename_component(prjName "${CMAKE_SOURCE_DIR}" NAME_WLE)
add_library ("${prjName}" SHARED
  "src/BotImpl.cpp"
)

target_compile_definitions("${prjName}" PUBLIC "CHARACTER_NAME=${CHARACTER_NAME}" "CHARACTER_CLASS=${CHARACTER_CLASS}")

# TODO: Add tests and install targets if needed.

target_include_directories("${prjName}" PUBLIC "../../src")
target_link_directories("${prjName}" PUBLIC "../../")
TARGET_LINK_LIBRARIES("${prjName}" Bot HttpWrapper)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET "${prjName}" PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET "${prjName}" PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries("${prjName}" "${CMAKE_THREAD_LIBS_INIT}")
endif()
set_property(TARGET "${prjName}" PROPERTY COMPILE_OPTIONS "-fPIC")