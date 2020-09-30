# src/

add_library(waveread waveread.hpp)
#add_executable (waveread_demo demo.cpp) 			# if a demo is made in the future.
#target_link_libraries (waveread_demo waveread)
set_target_properties(waveread PROPERTIES LINKER_LANGUAGE CXX)

# waveread uses std::thread and on g++ you need pthreads to use its implementation.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   find_package (Threads REQUIRED)
   target_link_libraries(waveread ${CMAKE_THREAD_LIBS_INIT})
endif()