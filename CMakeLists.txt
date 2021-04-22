cmake_minimum_required(VERSION 3.5)
project(si-benchmark)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

find_package(libspatialindex REQUIRED) # need to install from source with cmake
add_executable(benchmark_libspatialindex benchmark_libspatialindex.cpp)
target_link_libraries(benchmark_libspatialindex spatialindex)

find_package(SQLite3 REQUIRED)
add_executable(benchmark_sqlite3 benchmark_sqlite3.cpp)
target_include_directories(benchmark_sqlite3 PUBLIC ${SQLite3_INCLUDE_DIRS})
target_link_libraries(benchmark_sqlite3 ${SQLite3_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS date_time thread)
add_executable(benchmark_boost benchmark_boost.cpp)
target_include_directories(benchmark_boost PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(benchmark_boost ${Boost_LIBRARIES})
