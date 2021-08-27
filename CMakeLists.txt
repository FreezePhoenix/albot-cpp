# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -w -g -O3 -Wall)
project ("albot-cpp")
set(USE_TLS TRUE)
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

add_library(alclient-cpp SHARED
  "src/alclient-cpp.cpp"
  "src/BotSkeleton.cpp"
)
add_subdirectory(./lib/spdlog ./lib/spdlog)
add_subdirectory(./lib/TriangleManipulator ./lib/TriangleManipulator)
add_subdirectory(./lib/IXWebSocket ./lib/IXWebSocket)
set_property(TARGET HttpWrapper PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET spdlog PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET Bot PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET albot-cpp PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET alclient-cpp PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET HttpWrapper PROPERTY COMPILE_OPTIONS "-fPIC")
# TODO: Add tests and install targets if needed.

include_directories("./lib/TriangleManipulator/src")
include_directories("./lib/spdlog/include")
target_link_directories(albot-cpp PUBLIC "./")
target_include_directories(Bot PUBLIC "./lib/IXWebSocket")

TARGET_LINK_LIBRARIES(MapProcessing PUBLIC TriangleManipulator fmt)
TARGET_LINK_LIBRARIES(HttpWrapper PUBLIC PocoFoundation PocoNet spdlog PocoNetSSL)
TARGET_LINK_LIBRARIES(albot-cpp PUBLIC ${CMAKE_DL_LIBS} spdlog HttpWrapper MapProcessing TriangleManipulator triangle fmt)
TARGET_LINK_LIBRARIES(alclient-cpp PUBLIC ${CMAKE_DL_LIBS} HttpWrapper spdlog Bot)
TARGET_LINK_LIBRARIES(Bot PUBLIC ixwebsocket)

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
# if(ALBOT_DEBUG)
  set_property(TARGET albot-cpp PROPERTY COMPILE_OPTIONS "-ggdb")
  set_property(TARGET HttpWrapper PROPERTY COMPILE_OPTIONS "-ggdb")
  set_property(TARGET Bot PROPERTY COMPILE_OPTIONS "-ggdb")
# endif()