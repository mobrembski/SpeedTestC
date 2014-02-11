#cmake_minimum_required (VERSION 2.6)
#project (GSTTask1)
#find_package(PkgConfig)

#pkg_check_modules(GSTREAMER gstreamer-1.0)
#pkg_check_modules(GLIB glib-2.0)

#include_directories(${GSTREAMER_INCLUDE_DIRS})
#include_directories(${GLIB_INCLUDE_DIRS})
#link_directories(${GSTREAMER_LIBRARY_DIRS})
#link_directories(${GLIB_LIBRARY_DIRS})

add_definitions("-Wall")

#add_library(A GSTTaskUtils)
add_executable(SpeedTest.NET_C_Wrapper Speedtest.c)
#target_link_libraries(GSTTask1 ${GSTREAMER_LIBRARIES})
#target_link_libraries(GSTTask1 ${GLIB_LIBRARIES})
#target_link_libraries(SpeedTest.NET_C_Wrapper A)

install (TARGETS SpeedTest.NET_C_Wrapper DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
