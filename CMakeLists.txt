cmake_minimum_required (VERSION 3.5)
project (
  albot-cpp
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")


set(USE_TLS TRUE)
set(USE_OPEN_SSL TRUE)

set(SPDLOG_USE_STD_FORMAT OFF)
add_subdirectory(lib/libuv EXCLUDE_FROM_ALL)
# add_subdirectory(lib/spdlog lib/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(lib/IXWebSocket lib/IXWebSocket EXCLUDE_FROM_ALL)
# add_library(spdlog_HEADERS ALIAS spdlog_header_only)
add_subdirectory(lib/TriangleManipulator lib/TriangleManipulator EXCLUDE_FROM_ALL)


# Add source to this project's executable.

add_executable (albot-cpp
	"src/albot-cpp.cpp"
)

add_library(MapProcessing STATIC
  "src/MapProcessing/MapProcessing.cpp"
)

add_library(HttpWrapper STATIC
  "src/HttpWrapper.cpp"
)

add_library(Bot SHARED 
  "src/Bot.cpp"
  "src/BotSkeleton.cpp"
  "src/SocketWrapper.cpp"
)

add_library(alclient-cpp STATIC
  "src/alclient-cpp.cpp"
)

# TODO: Add tests and install targets if needed.
include_directories(include lib/uvw/src)
target_link_directories(albot-cpp PUBLIC "./")

target_link_libraries(
    MapProcessing
  PUBLIC
    TriangleManipulator
)

target_link_libraries(
    HttpWrapper 
  PUBLIC 
    spdlog
    PocoFoundation
    PocoNet
    PocoNetSSL
  PRIVATE
    MapProcessing
    fmt
)

target_link_libraries(
    Bot
  PUBLIC
    spdlog
    ixwebsocket
    uv
)

target_link_libraries(
    albot-cpp 
  PUBLIC
    TriangleManipulator
    ${CMAKE_DL_LIBS}
    HttpWrapper
)

target_link_libraries(
    alclient-cpp
  PUBLIC
    ${CMAKE_DL_LIBS}
    HttpWrapper
    Bot
)

set_property(TARGET albot-cpp PROPERTY LINK_OPTIONS "-rdynamic")