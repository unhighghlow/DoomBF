
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE BF_SOURCES "src/*.c")
file(GLOB_RECURSE BF_HEADERS "include/*.h")

add_executable(hackablebf ${BF_SOURCES} ${BF_HEADERS})
target_compile_options(hackablebf PRIVATE ${BF_COMPILE_OPTIONS})
target_include_directories(hackablebf
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#add_custom_command(TARGET hackablebf POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different hackablebf "${CMAKE_SOURCE_DIR}/bin/"
#    COMMENT "\n~~~~ Copying tools to bin directory"
#)