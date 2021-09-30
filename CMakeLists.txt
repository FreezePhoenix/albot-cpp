# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
set(POSITION_INDEPENDENT_CODE ON)
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -O3 -Wall -g)

set(USE_TLS TRUE)
set(USE_OPEN_SSL TRUE)

add_subdirectory(lib/spdlog lib/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(lib/TriangleManipulator lib/TriangleManipulator)

project ("albot-cpp")
add_subdirectory(lib/IXWebSocket lib/IXWebSocket)
# Add source to this project's executable.
add_executable (albot-cpp
	"src/albot-cpp.cpp"
  "src/JsonUtils.cpp"
)


add_library(MapProcessing STATIC
  "src/MapProcessing/MapProcessing.cpp"
  "src/MapProcessing/Objectifier.cpp"
  "src/MapProcessing/Writer.cpp"
)

add_library(HttpWrapper STATIC
  "src/HttpWrapper.cpp"
)

add_library(Bot SHARED 
  "src/Bot.cpp"
  "src/SocketWrapper.cpp"
)

add_library(alclient-cpp STATIC
  "src/alclient-cpp.cpp"
  "src/BotSkeleton.cpp"
)

# TODO: Add tests and install targets if needed.

include_directories("lib/spdlog/include" lib/IXWebSocket)
target_link_directories(albot-cpp PUBLIC "./")
target_include_directories(Bot PUBLIC "./lib/IXWebSocket")

target_link_libraries(MapProcessing PUBLIC TriangleManipulator)
target_link_libraries(HttpWrapper PUBLIC MapProcessing PocoFoundation PocoNet spdlog PocoNetSSL)
target_link_libraries(albot-cpp PUBLIC ${CMAKE_DL_LIBS} spdlog HttpWrapper MapProcessing TriangleManipulator triangle fmt ixwebsocket)
target_link_libraries(alclient-cpp PUBLIC ${CMAKE_DL_LIBS} HttpWrapper spdlog Bot)
target_link_libraries(Bot PUBLIC ixwebsocket)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET albot-cpp PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET HttpWrapper PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET Bot PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET albot-cpp PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
  set_property(TARGET HttpWrapper PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
  set_property(TARGET Bot PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(albot-cpp PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
  target_link_libraries(HttpWrapper PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
  target_link_libraries(Bot PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
endif()