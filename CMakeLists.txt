# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -w -g3 -O2)
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

add_library(HttpWrapper SHARED
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

add_subdirectory(./IXWebSocket ./IXWebSocket)
add_subdirectory(./TriangleManipulator ./TriangleManipulator)
set_property(TARGET Bot PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS "-fPIC")
set_property(TARGET HttpWrapper PROPERTY COMPILE_OPTIONS "-fPIC")
# TODO: Add tests and install targets if needed.

target_link_directories(albot-cpp PUBLIC "./")
target_include_directories(Bot PUBLIC "./IXWebSocket")
target_include_directories(MapProcessing PUBLIC "./TriangleManipulator/src")
target_include_directories(HttpWrapper PUBLIC "./TriangleManipulator/src")

TARGET_LINK_LIBRARIES(MapProcessing TriangleManipulator)
TARGET_LINK_LIBRARIES(HttpWrapper PocoFoundation PocoNet spdlog PocoNetSSL)
TARGET_LINK_LIBRARIES(albot-cpp ${CMAKE_DL_LIBS} spdlog HttpWrapper MapProcessing TriangleManipulator)
TARGET_LINK_LIBRARIES(alclient-cpp ${CMAKE_DL_LIBS} spdlog HttpWrapper Bot)
TARGET_LINK_LIBRARIES(Bot ixwebsocket)

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
  target_link_libraries(albot-cpp "${CMAKE_THREAD_LIBS_INIT}")
  target_link_libraries(HttpWrapper "${CMAKE_THREAD_LIBS_INIT}")
  target_link_libraries(Bot "${CMAKE_THREAD_LIBS_INIT}")
endif()
if(ALBOT_DEBUG)
  set_property(TARGET albot-cpp PROPERTY COMPILE_DEFINITIONS "-ggdb")
  set_property(TARGET HttpWrapper PROPERTY COMPILE_DEFINITIONS "-ggdb")
  set_property(TARGET Bot PROPERTY COMPILE_DEFINITIONS "-ggdb")
  endif()