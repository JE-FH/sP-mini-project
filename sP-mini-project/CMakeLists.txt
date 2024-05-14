# CMakeList.txt : CMake project for sP-mini-project, include source and define
# project specific logic here.
#

find_package(plplot CONFIG REQUIRED)

enable_testing()

# Add source to this project's executable.
add_executable (unit_tests "unit_tests.cpp" "library/stosim.cpp")
target_link_libraries(unit_tests PRIVATE doctest::doctest_with_main)
target_link_libraries(unit_tests PRIVATE libcoro)

add_executable (demo "demo.cpp" "library/stosim.cpp")
target_link_libraries(demo PRIVATE PLPLOT::plplotcxx)
target_link_libraries(demo PRIVATE libcoro)

add_executable(stosim_bm "benchmark.cpp" "library/stosim.cpp")
target_link_libraries(stosim_bm PRIVATE benchmark::benchmark)
target_link_libraries(stosim_bm PRIVATE libcoro)

set_property(TARGET unit_tests PROPERTY CXX_STANDARD 23)
set_property(TARGET demo PROPERTY CXX_STANDARD 23)
set_property(TARGET stosim_bm PROPERTY CXX_STANDARD 23)

# TODO: Add tests and install targets if needed.
