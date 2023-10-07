add_executable(main "${CMAKE_CURRENT_SOURCE_DIR}/DataFrame_test.cpp")

target_link_libraries(main 
    PRIVATE
        dataframe
        ${CONAN_LIBS_GTEST}
)
