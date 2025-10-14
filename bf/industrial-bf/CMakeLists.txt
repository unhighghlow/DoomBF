set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

if (WIN32)
  set(TCC_IBF_OUT "${CMAKE_BINARY_DIR}/tcc_ibf.exe")
  set(TCC_BLD_OUT "${CMAKE_BINARY_DIR}/tcc_bld.exe")
else()
  set(TCC_IBF_OUT "${CMAKE_BINARY_DIR}/tcc_ibf.bin")
  set(TCC_BLD_OUT "${CMAKE_BINARY_DIR}/tcc_bld.bin")
endif()

if (MINGW OR NOT(WIN32))
    add_executable(ibf ${CMAKE_CURRENT_SOURCE_DIR}/evaluator/main.c )
    target_compile_options(ibf PRIVATE -O3)
    add_custom_command(TARGET ibf POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ibf> "${CMAKE_SOURCE_DIR}/bin/")
    
    add_executable(bld ${CMAKE_CURRENT_SOURCE_DIR}/loader/loader.c )
    target_compile_options(bld PRIVATE -O3)
    add_custom_command(TARGET bld POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bld> "${CMAKE_SOURCE_DIR}/bin/")
    
endif()

add_custom_command(
    OUTPUT ${TCC_IBF_OUT}
    COMMAND chcp 65001
    COMMAND ${TCC_EXE} -m32 -o ${TCC_IBF_OUT} ${CMAKE_CURRENT_SOURCE_DIR}/evaluator/main.c -I${CMAKE_CURRENT_SOURCE_DIR} -I ${CMAKE_SOURCE_DIR}/bin/include -L ${CMAKE_SOURCE_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TCC_IBF_OUT} "${CMAKE_SOURCE_DIR}/bin/"
    COMMENT "\n~~~~ Building ${TCC_IBF_OUT} with tcc"
)

add_custom_command(
    OUTPUT ${TCC_BLD_OUT}
    COMMAND chcp 65001
    COMMAND ${TCC_EXE} -m32 -o ${TCC_BLD_OUT} ${CMAKE_CURRENT_SOURCE_DIR}/loader/loader.c -I${CMAKE_CURRENT_SOURCE_DIR} -I ${CMAKE_SOURCE_DIR}/bin/include -L ${CMAKE_SOURCE_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TCC_BLD_OUT} "${CMAKE_SOURCE_DIR}/bin/"
    COMMENT "\n~~~~ Building ${TCC_BLD_OUT} with tcc"
)

add_custom_target(
    industrialbf ALL
    DEPENDS ${TCC_IBF_OUT} ${TCC_BLD_OUT}
)

add_custom_target(clean_industrialbf ALL
    COMMAND ${CMAKE_COMMAND} -E rm -f "${TCC_IBF_OUT}"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${TCC_BLD_OUT}"
    COMMENT "\n~~~~ Remove old ibf/bld"
)

add_dependencies(industrialbf tcc clean_industrialbf)
