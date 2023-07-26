cmake_minimum_required (VERSION 3.5)
project ("albot-cpp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")


set(USE_TLS TRUE)
set(USE_OPEN_SSL TRUE)


set(SPDLOG_FMT_HO ON)
add_subdirectory(lib/libuv EXCLUDE_FROM_ALL)
add_subdirectory(lib/spdlog lib/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(lib/IXWebSocket lib/IXWebSocket EXCLUDE_FROM_ALL)
add_library(spdlog_HEADERS ALIAS spdlog_header_only)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(lib/TriangleManipulator lib/TriangleManipulator EXCLUDE_FROM_ALL)


# Add source to this project's executable.

add_library(IXWebSocket_HEADERS INTERFACE)
target_include_directories(IXWebSocket_HEADERS INTERFACE lib/IXWebSocket)

add_library(albot_HEADERS INTERFACE)
target_include_directories(albot_HEADERS INTERFACE include)

add_executable (albot-cpp
	"src/albot-cpp.cpp"
  "src/JsonUtils.cpp"
)

add_library(MapProcessing_HEADERS INTERFACE)
target_link_libraries(MapProcessing_HEADERS INTERFACE TriangleManipulator_HEADERS)

add_library(MapProcessing STATIC
  "src/MapProcessing/MapProcessing.cpp"
)

add_library(HttpWrapper STATIC
  "src/HttpWrapper.cpp"
)

add_library(Bot_HEADERS INTERFACE)
target_link_libraries(Bot_HEADERS INTERFACE IXWebSocket_HEADERS)

add_library(Bot SHARED 
  "src/Bot.cpp"
  "src/BotSkeleton.cpp"
  "src/SocketWrapper.cpp"
)

add_library(alclient-cpp STATIC
  "src/alclient-cpp.cpp"
)

# TODO: Add tests and install targets if needed.
include_directories(lib/IXWebSocket include lib lib/uvw/src)
target_link_directories(albot-cpp PUBLIC "./")
target_include_directories(Bot PUBLIC "./lib/IXWebSocket")

target_link_libraries(MapProcessing PUBLIC MapProcessing_HEADERS)
target_link_libraries(HttpWrapper PUBLIC MapProcessing_HEADERS spdlog_HEADERS PocoFoundation PocoNet PocoNetSSL)
target_link_libraries(Bot PUBLIC albot_HEADERS Bot_HEADERS spdlog_HEADERS ixwebsocket uv)
target_link_libraries(albot-cpp PUBLIC ${CMAKE_DL_LIBS} albot_HEADERS HttpWrapper MapProcessing TriangleManipulator spdlog Bot uv)
target_link_libraries(alclient-cpp PUBLIC ${CMAKE_DL_LIBS} albot_HEADERS HttpWrapper spdlog_HEADERS Bot)
set_property(TARGET albot-cpp PROPERTY LINK_OPTIONS "-Wl,--export-dynamic")
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET albot-cpp PROPERTY COMPILE_OPTIONS "-pthread -Wl,--export-dynamic")
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