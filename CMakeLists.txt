cmake_minimum_required(VERSION 3.5)

project(testtask_ivfs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_compile_options(-fsanitize=address,undefined)
#add_link_options(-fsanitize=address,undefined)
set(SRC src/main.cpp src/ivfs.cpp)
set(HR ivfs.h)
add_executable(testtask_ivfs ${SRC} ${HR})
target_include_directories(testtask_ivfs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS testtask_ivfs
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
