#supert_platform_executable(supert
#    supert.cpp
#)

project(tcc)

set(_TCC_Type WIN32)

add_executable(tcc ${_TCC_Type} tcc.c)
target_compile_definitions(tcc PRIVATE TCC_TARGET_PE)
target_compile_definitions(tcc PRIVATE TCC_TARGET_I386) 
target_compile_definitions(tcc PRIVATE ONE_SOURCE=1) 

#target_include_directories(tcc PRIVATE ${CMAKE_SOURCE_DIR}/extern)

set(_TCC_BinDir ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
    set_target_properties(tcc PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    set_property(TARGET tcc PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_custom_command(TARGET tcc POST_BUILD
    COMMAND chcp 65001
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/lib/libtcc1.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/crt1.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/crt1w.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/wincrt1.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/wincrt1w.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/dllcrt1.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/dllmain.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/win32/lib/chkstk.S
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -w -c ${CMAKE_SOURCE_DIR}/tcc/lib/bcheck.c
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/lib/alloca86.S
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -I ${CMAKE_SOURCE_DIR}/tcc/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include -I ${CMAKE_SOURCE_DIR}/tcc/win32/include/winapi -c ${CMAKE_SOURCE_DIR}/tcc/lib/alloca86-bt.S
    COMMAND ${_TCC_BinDir}\\tcc.exe -m32 -ar libtcc1-32.a libtcc1.o crt1.o crt1w.o wincrt1.o wincrt1w.o dllcrt1.o dllmain.o chkstk.o bcheck.o alloca86.o alloca86-bt.o
    WORKING_DIRECTORY "${_TCC_BinDir}"
    COMMENT "Compiling libc+crt with tcc.exe"
)




#add_custom_command(
#    TARGET tcc
#    PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ARGS ${_SUPERT_BinDir}/data
#)
