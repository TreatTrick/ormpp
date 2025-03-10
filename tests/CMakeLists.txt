set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
project(test_ormpp)
add_executable(${PROJECT_NAME}
        test_ormpp.cpp
        main.cpp
        )

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

if(ENABLE_PG)
        target_link_libraries(${PROJECT_NAME} ${PGSQL_LIBRARY} doctest)
elseif(ENABLE_MYSQL)
        target_link_libraries(${PROJECT_NAME} ${MYSQL_LIBRARY} doctest)
        if (MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
                set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MD")
        endif()

        # 定义要复制的特定DLL文件
        set(DLL_FILES
                "libcrypto-3-x64.dll"
                "libssl-3-x64.dll"
        )

        # 为每个文件添加一个复制命令
        foreach(DLL_FILE ${DLL_FILES})
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MYSQL_INCLUDE_DIR}/../bin/${DLL_FILE}"
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>
                        COMMENT "正在复制 ${DLL_FILE} 到可执行文件目录"
                )
        endforeach()

elseif(ENABLE_MARIADB)
        target_link_libraries(${PROJECT_NAME} ${MARIADB_LIBRARY} doctest)
elseif(ENABLE_SQLITE3)
        target_link_libraries(${PROJECT_NAME} sqlite3 doctest)
endif()