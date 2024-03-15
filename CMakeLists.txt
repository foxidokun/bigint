cmake_minimum_required(VERSION 3.27)
project(bigint)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-pedantic -Wextra -std=c++20)

# GTest section

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB LIBSRCS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
)

file(GLOB PLAYSRCS
    "${PROJECT_SOURCE_DIR}/play/*.cpp"
    "${PROJECT_SOURCE_DIR}/play/*.hpp"
)

file(GLOB TESTSRCS
    "${PROJECT_SOURCE_DIR}/test/*.cpp"
    "${PROJECT_SOURCE_DIR}/test/*.hpp"
)

add_library(bigint_lib STATIC ${LIBSRCS})

add_executable(playground ${PLAYSRCS})
target_include_directories(playground PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(playground PRIVATE bigint_lib)

# GTest Section

enable_testing()

add_executable(
  bigint_test
  ${TESTSRCS}
)

target_include_directories(bigint_test PRIVATE "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(
  bigint_test
  bigint_lib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(bigint_test)