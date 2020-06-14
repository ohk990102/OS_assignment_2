CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( "os-2" )
SET ( PROJECT_VERSION_MAJOR 0 )
SET ( PROJECT_VERSION_MINOR 1 )

file (GLOB SRC_FILES 
    "main.c"
    "${PROJECT_SOURCE_DIR}/utils/*.c"
    )

file (GLOB TEST_SRC_FILES
    "test/*.c")

SET ( SCHEDULER_LIST
    fcfs
    sjf
    rr
    )

foreach(SCHEDULER  ${SCHEDULER_LIST})
    add_executable(${SCHEDULER} ${SRC_FILES} "${SCHEDULER}/${SCHEDULER}.c")
endforeach()

foreach(TEST ${TEST_SRC_FILES})
    get_filename_component(TEST_NAME ${TEST} NAME_WE)
    add_executable(${TEST_NAME} ${TEST})
endforeach()