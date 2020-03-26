cmake_minimum_required(VERSION 3.9)

set (SRCS
    src/embedded-event.cpp
    src/embedded-container.cpp
    src/embedded-event-mutex.cpp
    src/embedded-event-task.cpp
    src/embedded-event-barrier.cpp
    src/embedded-event-registration.cpp
    src/embedded-event-link.cpp
)

if (ESP_PLATFORM)

    idf_component_register(
        SRCS ${SRCS}
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
        ${SRCS}
    )

    find_package(Threads)
    if(Threads_FOUND)
        if(CMAKE_USE_PTHREADS_INIT)
            target_compile_definitions(${PROJECT_NAME}
                PUBLIC EMBEDDED_EVENT_PTHREADS)
        else()
            set(CMAKE_CXX_STANDARD 11)
            set(CMAKE_CXX_STANDARD_REQUIRED ON)
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