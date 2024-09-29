cmake_minimum_required(VERSION 3.12)
project(HOCO_TESTS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3")

find_package(Boost REQUIRED)
message(STATUS "Boost version: ${Boost_VERSION}")

set(SchemeCollectModuleDIR SchemeCollectModule)
set(SchemesDIR ${SchemeCollectModuleDIR}/schemes)
set(SeuiturDIR ${SchemesDIR}/sequiturBasedEngine)
set(CompressModuleDIR CompressModule)
set(EvaluateModuleDIR EvaluateModule)

set(CompressModuleTarget test_CompressModule)
set(EvaluateModuleTarget test_EvaluateModule)
set(SchemeCollectModuleTarget test_SchemeCollectModule)

set(CompressModuleSRCS test_CompressModule.cpp)
set(EvaluateModuleSRCS test_EvaluateModule.cpp)
set(SchemeCollectModuleSRCS test_SchemeCollectModule.cpp)

set(HDRS ${CompressModuleDIR}/CompressModule.hpp Utils/utils.hpp ${SchemesDIR}/TADOC_HC.hpp ${SchemesDIR}/LZW_HC.hpp ${SchemesDIR}/RLE_HC.hpp ${SchemesDIR}/sequitur_wrapper.hpp)

file(GLOB CompressModuleSRCS ${CMAKE_CURRENT_SOURCE_DIR}/test_CompressModule.cpp)
file(GLOB EvaluateModuleSRCS ${CMAKE_CURRENT_SOURCE_DIR}/test_EvaluateModule.cpp)
file(GLOB SchemeCollectModuleSRCS ${CMAKE_CURRENT_SOURCE_DIR}/test_SchemeCollectModule.cpp)

# Define the data file path as a preprocessor definition
add_definitions(-DPROJECT_ROOT_PATH="${CMAKE_SOURCE_DIR}/")
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
add_definitions(-DPARENT_DIR="${PARENT_DIR}/")

add_executable(${CompressModuleTarget} ${CompressModuleSRCS} ${SequiturOBJS})
add_executable(${EvaluateModuleTarget} ${EvaluateModuleSRCS} ${SequiturOBJS})
add_executable(${SchemeCollectModuleTarget} ${SchemeCollectModuleSRCS} ${SequiturOBJS})

target_include_directories(${CompressModuleTarget} PRIVATE ${PROJECT_INCLUDE_DIRS})
target_include_directories(${EvaluateModuleTarget} PRIVATE ${PROJECT_INCLUDE_DIRS})
target_include_directories(${SchemeCollectModuleTarget} PRIVATE ${PROJECT_INCLUDE_DIRS})

# Generate object file names from source files
file(GLOB SEQUITUR_SRCS "${SeuiturDIR}/*.cc" "${SeuiturDIR}/*.c")
file(GLOB SEQUITUR_HDRS "${SeuiturDIR}/*.h")
add_library(SequiturLib OBJECT ${SEQUITUR_SRCS} ${SEQUITUR_HDRS})
target_include_directories(SequiturLib PUBLIC ${SeuiturDIR})


target_link_libraries(${CompressModuleTarget} PRIVATE ${PROJECT_LIBRARIES} $<TARGET_OBJECTS:SequiturLib>)
target_link_libraries(${EvaluateModuleTarget} PRIVATE ${PROJECT_LIBRARIES} $<TARGET_OBJECTS:SequiturLib>)
target_link_libraries(${SchemeCollectModuleTarget} PRIVATE ${PROJECT_LIBRARIES} $<TARGET_OBJECTS:SequiturLib>)

enable_testing()
add_test(NAME CompressModuleTest COMMAND ${CompressModuleTarget})
add_test(NAME EvaluateModuleTest COMMAND ${EvaluateModuleTarget})
add_test(NAME SchemeCollectModuleTest COMMAND ${SchemeCollectModuleTarget})
