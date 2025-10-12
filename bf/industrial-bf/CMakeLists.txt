
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

#add_executable(ibf src/main.c)
#target_compile_options(ibf PRIVATE ${BF_COMPILE_OPTIONS})
#add_executable(bfm src/macros.c)
#target_compile_options(bfm PRIVATE ${BF_COMPILE_OPTIONS})
#add_executable(bld src/loader.c)
#target_compile_options(bld PRIVATE ${BF_COMPILE_OPTIONS})

add_custom_command(
    OUTPUT ibf
    COMMAND chcp 65001
    COMMAND ${TCC_EXE} -m32 -o ibf.exe ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c -I${CMAKE_CURRENT_SOURCE_DIR} -I ${CMAKE_SOURCE_DIR}/bin/include -L ${CMAKE_SOURCE_DIR}/bin
    COMMENT "\n~~~~ Building industrial-bf\\ibf.exe with tcc"
)

add_custom_command(
    OUTPUT bfm
    COMMAND chcp 65001
    COMMAND ${TCC_EXE} -m32 -o bfm.exe ${CMAKE_CURRENT_SOURCE_DIR}/src/macros.c -I${CMAKE_CURRENT_SOURCE_DIR} -I ${CMAKE_SOURCE_DIR}/bin/include -L ${CMAKE_SOURCE_DIR}/bin
    COMMENT "\n~~~~ Building industrial-bf\\bfm.exe with tcc"
)

add_custom_command(
    OUTPUT bld
    COMMAND chcp 65001
    COMMAND ${TCC_EXE} -m32 -o bld.exe ${CMAKE_CURRENT_SOURCE_DIR}/src/loader.c -I${CMAKE_CURRENT_SOURCE_DIR} -I ${CMAKE_SOURCE_DIR}/bin/include -L ${CMAKE_SOURCE_DIR}/bin
    COMMENT "\n~~~~ Building industrial-bf\\bld.exe with tcc"
)

add_custom_target(
    industrialbf ALL
    DEPENDS ibf bld bfm
)
add_dependencies(industrialbf tcc)
#add_custom_command(TARGET industrialbf POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different bld "${CMAKE_SOURCE_DIR}/bin/"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different bfm "${CMAKE_SOURCE_DIR}/bin/"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different ibf "${CMAKE_SOURCE_DIR}/bin/"
#    COMMENT "\n~~~~ Copying tools to bin directory"
#)