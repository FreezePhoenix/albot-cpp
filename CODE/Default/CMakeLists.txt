# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)

add_definitions(-std=c++17)

project ("albot-cpp")

# Add source to this project's executable.
add_library (albot-code SHARED
	"src/Bot.cpp"
	"../../src/JsonUtils.cpp"
	"src/SocketWrapper.cpp"
)

# TODO: Add tests and install targets if needed.
TARGET_LINK_LIBRARIES(albot-code sioclient ixwebsocket)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET albot-code PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET albot-code PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(albot-code "${CMAKE_THREAD_LIBS_INIT}")
endif()
set_property(TARGET albot-code PROPERTY COMPILE_OPTIONS "-fPIC")