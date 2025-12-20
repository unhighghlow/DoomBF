set(OUT "${CMAKE_BINARY_DIR}/hello.b")

add_custom_command(
        OUTPUT ${OUT}
        #COMMAND chcp 65001
        COMMAND ${TCC_EXE} -o ${OUT} ${CMAKE_SOURCE_DIR}/test/hello.c -bf -nostdlib -I${CMAKE_SOURCE_DIR}/bin/include -L${CMAKE_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OUT}" "${CMAKE_SOURCE_DIR}/bin/"
        DEPENDS hello.c
        COMMENT "\n~~~~ Building hello.b with tcc"
    )

add_custom_target(
        hello ALL
        DEPENDS ${OUT}
    )

add_dependencies(hello tcc)
