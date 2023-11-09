cmake_minimum_required(VERSION 3.2)
project(HomeWorkData-4-1)

add_subdirectory("E:\\Stady\\Netology_cpp\\HomeWorks\\HomeWorkData\\libpqxx-7.8.1" libpqxx-build)

add_executable(HomeWorkData-4-1 HomeWorkData-4-1.cpp)

target_compile_features(HomeWorkData-4-1 PRIVATE cxx_std_17)

target_link_libraries(HomeWorkData-4-1 PRIVATE pqxx)