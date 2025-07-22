find_package(Catch2 3 REQUIRED)
find_package(spdlog REQUIRED)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS include/*.h include/*.hpp src/*.cpp)
add_executable(test ${srcs})
target_include_directories(test PUBLIC include)
target_link_libraries(test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

# add_executable(test_vector ./src/test_vector.cpp ${srcs})
# target_include_directories(test PUBLIC include)
# target_link_libraries(test_vector PRIVATE Catch2::Catch2WithMain)