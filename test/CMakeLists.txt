# test/

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1
)

FetchContent_MakeAvailable(Catch2)

include(ExternalData)
set(ExternalData_URL_TEMPLATES "ExternalDataCustomScript://httpsFetchExtract/https://billguastalla.com/binaries/wavereader/github_resources/samples.zip")
set(ExternalData_CUSTOM_SCRIPT_httpsFetchExtract ${CMAKE_SOURCE_DIR}/cmake/httpsFetchExtract.cmake)

#set(ExternalData_CUSTOM_SCRIPT "ExternalDataCustomScript://<key>/<loc>")

#ExternalData_Add_Target(testData)
#ExternalData_Expand_Arguments(testData sampleData DATA{samples.zip})

include_directories(${WAVEIO_SOURCE_DIR}/src)
#file(WRITE samples audioSamples)

add_executable(waveioTests test.cpp)
target_link_libraries(waveioTests Catch2::Catch2)
#add_test(NAME MyTest COMMAND waveioTests)
ExternalData_Add_Test(waveioTestAssets
   NAME MyTest
   COMMAND waveIOTests DATA{samples.zip})
ExternalData_Add_Target(waveioTestAssets)


#include(CTest)
#include(Catch)
#catch_discover_tests(foo)
