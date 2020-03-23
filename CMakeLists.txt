cmake_minimum_required(VERSION 3.8)

if (ESP_PLATFORM)

    target_compile_definitions(${PROJECT_NAME}
        PUBLIC ESP_PLATFORM)

    idf_component_register(
        SRCS "src/embedded-event.cpp"
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
    )

    # Support testing
    enable_testing()
    add_subdirectory(test)

endif()