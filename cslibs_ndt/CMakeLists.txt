cmake_minimum_required(VERSION 2.8.3)
project(cslibs_ndt)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/cslibs_ndt_enable_c++11.cmake)
include(cmake/cslibs_ndt_extras.cmake)
include(cmake/cslibs_ndt_openmp.cmake)
include(cmake/cslibs_ndt_show_headers.cmake)
include(cmake/cslibs_ndt_add_unit_test_gtest.cmake)

find_package(catkin REQUIRED COMPONENTS
    cslibs_math
    cslibs_indexed_storage
    cslibs_utility
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES 
  CATKIN_DEPENDS cslibs_math cslibs_utility cslibs_indexed_storage
  CFG_EXTRAS     cslibs_ndt_enable_c++11.cmake
                 cslibs_ndt_extras.cmake
                 cslibs_ndt_show_headers.cmake
                 cslibs_ndt_add_unit_test_gtest.cmake
                 cslibs_ndt_openmp.cmake
)

include_directories(
    include
    ${catkin_INCLUDE_DIRS}
)

cslibs_ndt_show_headers()

install(DIRECTORY include/${PROJECT_NAME}/
        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
