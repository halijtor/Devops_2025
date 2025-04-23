cmake_minimum_required(VERSION 3.10)
project(DeskCalc LANGUAGES CXX)

set(PUBLIC_INCLUDE "${CMAKE_SOURCE_DIR}/include")

set(MATH_PARSER_SRC
    src/Parser.cpp
    src/TokenStream.cpp
    src/SymbolTable.cpp
    src/Function.cpp
    src/SymbolGuard.cpp
    src/math_util.cpp
)

set(CALC_SRC
    src/main.cpp
    src/Calculator.cpp
)

add_library(MathParser ${MATH_PARSER_SRC})
target_include_directories(MathParser
    PUBLIC
        ${PUBLIC_INCLUDE}
)

add_executable(DeskCalc ${CALC_SRC})
target_link_libraries(DeskCalc PUBLIC MathParser)
target_include_directories(DeskCalc
    PRIVATE
        ${PUBLIC_INCLUDE}
)

install(TARGETS MathParser DeskCalc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY ${PUBLIC_INCLUDE}/
    DESTINATION include
)

