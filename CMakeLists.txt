cmake_minimum_required(VERSION 3.20)

project(cxx_koans)

find_package(cpprestsdk REQUIRED)

file(GLOB SOURCES
    source/main.cpp
    source/parser/*.cpp
    source/server/*.cpp
)
add_executable(cxx-koan-server ${SOURCES})

target_include_directories(cxx-koan-server PUBLIC
    source/parser
    source/server
)

target_link_libraries(cxx-koan-server PRIVATE
    Boost::thread Boost::filesystem
    cpprestsdk::cpprest
)