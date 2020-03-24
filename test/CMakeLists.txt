cmake_minimum_required(VERSION 3.8)

include_directories(../inc)

file(GLOB sources
    "../src/*.cpp"
)

# No threading
add_executable ("${PROJECT_NAME}_no_threading" unit_test.cpp ${sources})
add_test(NAME "${PROJECT_NAME}_no_threading" COMMAND "${PROJECT_NAME}_no_threading")
target_compile_definitions("${PROJECT_NAME}_no_threading"
    PUBLIC EMBEDDED_EVENT_NO_THREADING)

# Pthreads
find_package(Threads)
if(CMAKE_USE_PTHREADS_INIT)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    add_executable ("${PROJECT_NAME}_pthreads" unit_test.cpp ${sources})
    add_test(NAME "${PROJECT_NAME}_pthreads" COMMAND "${PROJECT_NAME}_pthreads")
    target_compile_definitions("${PROJECT_NAME}_pthreads"
        PUBLIC EMBEDDED_EVENT_PTHREADS)
else()
    message(SEND_ERROR "Pthreads not found")
endif()

# CPP11
if(Threads_FOUND)
    add_executable ("${PROJECT_NAME}_cpp11" unit_test.cpp ${sources})
    add_test(NAME "${PROJECT_NAME}_cpp11" COMMAND "${PROJECT_NAME}_cpp11")
    target_compile_definitions("${PROJECT_NAME}_cpp11"
        PUBLIC EMBEDDED_EVENT_CPP11)
endif()

# OpenMP
find_package(OpenMP)
if(OpenMP_FOUND)
    add_executable ("${PROJECT_NAME}_omp" unit_test.cpp ${sources})
    add_test(NAME "${PROJECT_NAME}_omp" COMMAND "${PROJECT_NAME}_omp")
    target_compile_definitions("${PROJECT_NAME}_omp"
        PUBLIC EMBEDDED_EVENT_OMP)
endif()