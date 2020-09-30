# src/

add_library(waveread waveread.hpp)
#add_executable (waveread_demo demo.cpp) 			# if a demo is made in the future.
#target_link_libraries (waveread_demo waveread)
set_target_properties(waveread PROPERTIES LINKER_LANGUAGE CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   target_link_libraries(waveioTests PRIVATE Threads::Threads)
endif()
