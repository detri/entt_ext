FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        GIT_SHALLOW 1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(GTest::Main ALIAS gtest_main)

target_compile_features(gtest PUBLIC cxx_std_17)
set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")

target_compile_features(gtest_main PUBLIC cxx_std_17)
set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")

target_compile_features(gmock PUBLIC cxx_std_17)
set_target_properties(gmock PROPERTIES CXX_CLANG_TIDY "")

target_compile_features(gmock_main PUBLIC cxx_std_17)
set_target_properties(gmock_main PROPERTIES CXX_CLANG_TIDY "")

include_directories($<TARGET_PROPERTY:entt_ext,INTERFACE_INCLUDE_DIRECTORIES>)
add_compile_options($<TARGET_PROPERTY:entt_ext,INTERFACE_COMPILE_OPTIONS>)

add_executable(test_entt_ext
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/test/src/relationship_tests.cpp>
        $<BUILD_INTERFACE:${entt_ext_SOURCE_DIR}/test/src/system_tests.cpp>
)

target_link_libraries(test_entt_ext PRIVATE EnTT::EnTT entt_ext gtest_main)
