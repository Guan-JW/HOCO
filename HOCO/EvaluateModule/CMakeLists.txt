cmake_minimum_required(VERSION 3.12)

# Set project information
project(EvaluateModule)

# Configure build options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Specify source files (cpp) and header files (hpp)
set(SRCS
    evaluate.cpp
)

set(HDRS
    EvaluateModule.hpp,
    ../SchemeCollectModule/schemes/include_list.txt,
    ../Utils/utils.hpp
)

# Specify the directory for Sequitur sources
set(SRCDIR ../SchemeCollectModule/schemes/sequiturBasedEngine)

# Generate object file names from source files
file(GLOB SEQUITUR_SRCS "${SRCDIR}/*.cc" "${SRCDIR}/*.c")
file(GLOB SEQUITUR_HDRS "${SRCDIR}/*.h")

# Add a library for the Sequitur sources
add_library(SequiturLib OBJECT ${SEQUITUR_SRCS} ${SEQUITUR_HDRS})
target_include_directories(SequiturLib PUBLIC ${SRCDIR})

# Specify the target executable name
set(TARGET EvaluateModule)

# Define the data file path as a preprocessor definition
add_definitions(-DPROJECT_ROOT_PATH="${CMAKE_SOURCE_DIR}/")
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
add_definitions(-DPARENT_DIR="${PARENT_DIR}/")

# Rule to build the target executable
add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET} PRIVATE $<TARGET_OBJECTS:SequiturLib>)

# Clean rule to remove generated files
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove ${TARGET}
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/*.o
)
