add_executable(example1 example1.c)
target_link_libraries(example1 PRIVATE AB_vector)
add_test(AB_vector.example1 example1)
