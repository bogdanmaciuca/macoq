include(CTest)
enable_testing()

add_compile_options(-O0)

function(macoq_add_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    target_link_libraries(${TEST_NAME} PRIVATE macoq)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

macoq_add_test(dummy)
macoq_add_test(mutex)
macoq_add_test(cond_var)
macoq_add_test(semaphore)
macoq_add_test(rwlock)
macoq_add_test(barrier)
macoq_add_test(producer_consumer)
