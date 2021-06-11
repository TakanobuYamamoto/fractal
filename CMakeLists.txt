cmake_minimum_required(VERSION 2.8)

project(fractal)

add_library(fractal SHARED src/fractal.cpp)
target_include_directories(fractal PRIVATE include)
target_link_libraries(fractal pthread)

include(GNUInstallDirs)
install(TARGETS fractal
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


add_executable(system_test example/system_test.cpp)
target_include_directories(system_test PRIVATE include)
target_link_libraries(system_test fractal)

add_executable(wav_test example/wav_test.cpp)
target_include_directories(wav_test PRIVATE include)
target_link_libraries(wav_test fractal)
