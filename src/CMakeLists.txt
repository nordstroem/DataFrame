target_sources(dataframe 
    PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/BooleanExpression.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DataFrame.cpp"
)

add_executable(main "${CMAKE_CURRENT_SOURCE_DIR}/DataFrame_test.cpp")

target_link_libraries(main 
    PRIVATE
        dataframe
        ${CONAN_LIBS_GTEST}
)
