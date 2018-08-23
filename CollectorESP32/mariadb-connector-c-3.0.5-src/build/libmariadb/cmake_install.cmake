# Install script for directory: C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/libmariadb

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/mariadb-connector-c")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/Debug/mariadbclient.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/Release/mariadbclient.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/MinSizeRel/mariadbclient.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/RelWithDebInfo/mariadbclient.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xSharedLibrariesx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/Debug/libmariadb.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/Release/libmariadb.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/MinSizeRel/libmariadb.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/RelWithDebInfo/libmariadb.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xSharedLibrariesx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE SHARED_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/Debug/libmariadb.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE SHARED_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/Release/libmariadb.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE SHARED_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/MinSizeRel/libmariadb.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE SHARED_LIBRARY FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/RelWithDebInfo/libmariadb.dll")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/mariadb" TYPE FILE FILES "C:/Users/user/Desktop/mariadbclient/MariaDbClient/MariaDbClient/mariadb-connector-c-3.0.5-src/build/libmariadb/${CMAKE_INSTALL_CONFIG_NAME}/libmariadb.pdb")
endif()

