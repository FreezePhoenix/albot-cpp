# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++17 -w)
project ("albot-cpp")
# Add source to this project's executable.
add_executable (albot-cpp
	"src/albot-cpp.cpp"
  "src/JsonUtils.cpp"
  "src/MapProcessing.cpp"
  "src/HttpWrapper.cpp"
)
# TODO: Add tests and install targets if needed.

TARGET_LINK_LIBRARIES(albot-cpp PocoFoundation PocoNet ${CMAKE_DL_LIBS} spdlog PocoNetSSL)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET albot-cpp PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET albot-cpp PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(albot-cpp "${CMAKE_THREAD_LIBS_INIT}")
endif()