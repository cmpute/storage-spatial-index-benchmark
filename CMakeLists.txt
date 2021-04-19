cmake_minimum_required(VERSION 3.5)
project(si-benchmark)

include_directories(${CMAKE_SOURCE_DIR})
find_package(libspatialindex REQUIRED) # need to install from source with cmake
find_package(SQLite3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(benchmark_libspatialindex benchmark_libspatialindex.cpp)
target_link_libraries(benchmark_libspatialindex spatialindex)

add_executable(benchmark_sqlite3 benchmark_sqlite3.cpp)
target_include_directories(benchmark_sqlite3 PUBLIC ${SQLite3_INCLUDE_DIRS})
target_link_libraries(benchmark_sqlite3 ${SQLite3_LIBRARIES})
