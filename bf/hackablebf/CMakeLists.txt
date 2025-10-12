cmake_minimum_required(VERSION 3.15)
project(HackableBrainfuck VERSION 1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Для редакторов
set(CMAKE_C_STANDARD 11)

# Автоматический поиск исходников
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "include/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Подключение заголовков (важно для IDE)
target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(
        ${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
endif()
