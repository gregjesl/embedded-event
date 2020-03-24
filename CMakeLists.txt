cmake_minimum_required(VERSION 3.9)

if (ESP_PLATFORM)

    idf_component_register(
        SRCS src/embedded-event.cpp src/embedded-container.cpp src/embedded-event-mutex.cpp
        INCLUDE_DIRS "inc"
    )

else()

    project(embedded-event)

    if(MSVC)
        # ignore warnings about scanf
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_compile_options(/W4 /WX)
    else()
        add_compile_options(-Wall -Wextra -Werror)
    endif()

    include_directories ("${PROJECT_SOURCE_DIR}/inc")

    add_library(${PROJECT_NAME}
        src/embedded-event.cpp
        src/embedded-container.cpp
        src/embedded-event-mutex.cpp
    )

    find_package(Threads)
    if(Threads_FOUND)
        if(CMAKE_USE_PTHREADS_INIT AND THREADS_PREFER_PTHREAD_FLAG)
            target_compile_definitions(${PROJECT_NAME}
                PUBLIC EMBEDDED_EVENT_PTHREADS)
        else()
            target_compile_definitions(${PROJECT_NAME}
                PUBLIC EMBEDDED_EVENT_CPP11)
        endif()
    else()
        target_compile_definitions(${PROJECT_NAME}
            PUBLIC EMBEDDED_EVENT_NO_THREADING)
    endif()

    target_compile_definitions("${PROJECT_NAME}"
        PUBLIC EMBEDDED_EVENT_NO_THREADING)

    # Support testing
    enable_testing()
    add_subdirectory(test)

endif()