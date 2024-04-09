# CMakeList.txt : CMake project for sP-mini-project, include source and define
# project specific logic here.
#

include(doctest.cmake)
enable_testing()

# Add source to this project's executable.
add_executable (unit_tests "unit_tests.cpp" "library/stosim.cpp")
target_link_libraries(unit_tests PRIVATE doctest::doctest_with_main)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET unit_tests PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
