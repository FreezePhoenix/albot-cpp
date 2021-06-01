# Install script for directory: /home/linuxlite/shared/albot-cpp/IXWebSocket

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/linuxlite/shared/albot-cpp/CODE/IXWebSocket/libixwebsocket.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ixwebsocket" TYPE FILE FILES
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXBench.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXCancellationRequest.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXConnectionState.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXDNSLookup.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXExponentialBackoff.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXGetFreePort.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXGzipCodec.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXHttp.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXHttpClient.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXHttpServer.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXNetSystem.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXProgressCallback.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSelectInterrupt.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSelectInterruptFactory.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSelectInterruptPipe.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSetThreadName.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSocket.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSocketConnect.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSocketFactory.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSocketServer.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSocketTLSOptions.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXStrCaseCompare.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXUdpSocket.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXUniquePtr.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXUrlParser.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXUuid.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXUtf8Validator.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXUserAgent.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocket.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketCloseConstants.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketCloseInfo.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketErrorInfo.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketHandshake.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketHandshakeKeyGen.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketHttpHeaders.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketInitResult.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketMessage.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketMessageType.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketOpenInfo.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketPerMessageDeflate.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketPerMessageDeflateCodec.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketPerMessageDeflateOptions.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketProxyServer.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketSendInfo.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketServer.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketTransport.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXWebSocketVersion.h"
    "/home/linuxlite/shared/albot-cpp/IXWebSocket/ixwebsocket/IXSocketOpenSSL.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-config.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-config.cmake"
         "/home/linuxlite/shared/albot-cpp/CODE/IXWebSocket/CMakeFiles/Export/lib/cmake/ixwebsocket/ixwebsocket-config.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-config-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-config.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket" TYPE FILE FILES "/home/linuxlite/shared/albot-cpp/CODE/IXWebSocket/CMakeFiles/Export/lib/cmake/ixwebsocket/ixwebsocket-config.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket" TYPE FILE FILES "/home/linuxlite/shared/albot-cpp/CODE/IXWebSocket/CMakeFiles/Export/lib/cmake/ixwebsocket/ixwebsocket-config-debug.cmake")
  endif()
endif()

