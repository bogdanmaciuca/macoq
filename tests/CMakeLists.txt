include(CTest)
enable_testing()

function(macoq_add_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

macoq_add_test(dummy)


