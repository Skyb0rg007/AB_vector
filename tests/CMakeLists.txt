add_executable(example1 example1.c)
target_link_libraries(example1 PRIVATE AB_vector)
add_test(AB_vector.example1 example1)

add_executable(example2 example2.c)
target_link_libraries(example2 PRIVATE AB_vector)
add_test(AB_vector.example2 example2)
