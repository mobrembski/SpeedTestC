add_definitions("-g -std=c99 -Wall")

add_executable(SpeedTest.NET_C_Wrapper Speedtest.c http.c url.c SpeedtestConfig.c SpeedtestServers.c)
target_link_libraries(SpeedTest.NET_C_Wrapper m)

install (TARGETS SpeedTest.NET_C_Wrapper DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
