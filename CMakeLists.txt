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
)

add_library(HttpWrapper SHARED
  "src/HttpWrapper.cpp"
)

add_library(Bot SHARED
  "src/Bot.cpp"
)

set_property(TARGET Bot PROPERTY COMPILE_OPTIONS "-fPIC")
# TODO: Add tests and install targets if needed.
target_link_directories(albot-cpp PUBLIC "./")
TARGET_LINK_LIBRARIES(HttpWrapper PocoFoundation PocoNet spdlog PocoNetSSL)
TARGET_LINK_LIBRARIES(albot-cpp ${CMAKE_DL_LIBS} spdlog HttpWrapper)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET albot-cpp PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET albot-cpp PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(albot-cpp "${CMAKE_THREAD_LIBS_INIT}")
endif()