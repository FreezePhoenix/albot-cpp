cmake_minimum_required (VERSION 3.5)
project ("BotImpl")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(USE_TLS TRUE)
set(USE_OPEN_SSL TRUE)

set(SPDLOG_USE_STD_FORMAT ON)

add_subdirectory(../../lib/TriangleManipulator ./build/TriangleManipulator EXCLUDE_FROM_ALL)
add_subdirectory(../../lib/spdlog ./build/spdlog EXCLUDE_FROM_ALL)

if(NOT DEFINED CHARACTER_NAME_STRING)
  set(CHARACTER_NAME_STRING "DEFAULT")
endif()

add_library(spdlog_HEADERS ALIAS spdlog_header_only)

get_filename_component(prjName "${CMAKE_SOURCE_DIR}" NAME_WLE)

add_library(${prjName}_HEADERS INTERFACE)
target_include_directories(${prjName}_HEADERS INTERFACE . ../../include ../../ ../../lib/uvw/src ../../lib/IXWebSocket)
target_link_libraries(${prjName}_HEADERS INTERFACE INTERFACE TriangleManipulator_HEADERS spdlog_HEADERS)

add_library (${prjName}Targeter OBJECT
  "src/Targeter.cpp"
)

add_library (${prjName}Functions OBJECT
  "src/Functions.cpp"
)

add_library (${prjName}ArmorManager OBJECT
  "src/ArmorManager.cpp"
)
add_library (${prjName}SkillHelper OBJECT
  "src/SkillHelper.cpp"
)

target_link_libraries(${prjName}Targeter PUBLIC ${prjName}_HEADERS)
target_link_libraries(${prjName}Functions PUBLIC ${prjName}_HEADERS)
target_link_libraries(${prjName}ArmorManager PUBLIC ${prjName}_HEADERS)
target_link_libraries(${prjName}SkillHelper PUBLIC ${prjName}_HEADERS)


foreach(C_NAME_STRING C_NAME C_CLASS IN ZIP_LISTS CHARACTER_NAME_STRING CHARACTER_NAME CHARACTER_CLASS)
  add_library (${prjName}_${C_NAME_STRING} SHARED
    "src/BotImpl.cpp"
  )
  target_compile_definitions(${prjName}_${C_NAME_STRING} PUBLIC ${NAME_DEFINITIONS} "CHARACTER_NAME=${C_NAME}" "CHARACTER_CLASS=0${C_CLASS}")
  target_link_directories(${prjName}_${C_NAME_STRING} PUBLIC ../../)
  target_link_libraries(${prjName}_${C_NAME_STRING} PUBLIC ${prjName}_HEADERS ${prjName}Targeter ${prjName}Functions ${prjName}SkillHelper ${prjName}ArmorManager Bot uv)
endforeach()