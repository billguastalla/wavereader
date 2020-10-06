# test/

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1
)

FetchContent_MakeAvailable(Catch2)


include(ExternalData)
set(ExternalData_URL_TEMPLATES "https://billguastalla.com/binaries/wavereader/github_resources/samples.tar")

#set(ExternalData_CUSTOM_SCRIPT "ExternalDataCustomScript://<key>/<loc>")

#ExternalData_Add_Target(testData)
#ExternalData_Expand_Arguments(testData sampleData DATA{samples.zip})

include_directories(${WAVEIO_SOURCE_DIR}/src)
#file(WRITE samples audioSamples)

add_executable(waveioTests test.cpp)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   target_link_libraries(waveioTests PUBLIC Catch2::Catch2 PRIVATE Threads::Threads)
else()
   target_link_libraries(waveioTests PUBLIC Catch2::Catch2)
endif()

#add_test(NAME MyTest COMMAND waveioTests)
#ExternalData_Add_Test(waveioTestAssets
#   NAME MyTest
#   COMMAND waveioTests DATA{samples.tar})
ExternalData_Expand_Arguments(waveioTestAssets SAMPLES_PATH DATA{samples.tar})

add_dependencies(waveioTests waveioTestAssets)

set(ASSETS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${ASSETS_DIRECTORY})
set(ExternalData_BINARY_ROOT ${ASSETS_DIRECTORY})

add_custom_command(TARGET waveioTests
                   POST_BUILD
                   COMMAND 
                   echo "${CMAKE_COMMAND} -E copy ${SAMPLES_PATH} ${ASSETS_DIRECTORY}/samples.tar" & 
                   ${CMAKE_COMMAND} -E copy ${SAMPLES_PATH} ${ASSETS_DIRECTORY}/samples.tar & 
                   ${CMAKE_COMMAND} -E chdir ${ASSETS_DIRECTORY} tar vxf samples.tar
                   COMMENT "Extracting test assets...")

ExternalData_Add_Target(waveioTestAssets)



#find_package(Catch2 REQUIRED)
#include(CTest)
include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
catch_discover_tests(waveioTests)
