# Code coverage configuration
# Usage: setup_coverage_targets(test_target)

function(setup_coverage_targets TARGET_NAME)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Coverage not supported for ${CMAKE_CXX_COMPILER_ID}")
        return()
    endif()
    
    # Coverage flags
    target_compile_options(${TARGET_NAME}
        PRIVATE
            -g
            -O0
            -fprofile-arcs
            -ftest-coverage
            --coverage
    )
    
    target_link_options(${TARGET_NAME}
        PRIVATE
            --coverage
    )
    
    # Find coverage tools
    find_program(GCOV_TOOL gcov)
    find_program(LCOV_TOOL lcov)
    find_program(GENHTML_TOOL genhtml)
    
    if(GCOV_TOOL AND LCOV_TOOL AND GENHTML_TOOL)
        # Collect coverage data
        add_custom_target(coverage-collect
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${LCOV_TOOL} --directory ${CMAKE_BINARY_DIR} --capture 
                --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
                --rc lcov_branch_coverage=1
            COMMAND ${LCOV_TOOL} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info
                "/usr/*" "${CMAKE_BINARY_DIR}/_deps/*" "${CMAKE_SOURCE_DIR}/tests/*"
                --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
                --rc lcov_branch_coverage=1
            DEPENDS ${TARGET_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Collecting coverage data"
        )
        
        # Generate HTML report
        add_custom_target(coverage-report
            COMMAND ${GENHTML_TOOL} --prefix ${CMAKE_SOURCE_DIR}
                ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage/html
                --title "GPS Pipeline Coverage Report"
                --legend --branch-coverage --function-coverage
            DEPENDS coverage-collect
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage HTML report"
        )
        
        # Open report in browser
        add_custom_target(coverage-view
            COMMAND ${CMAKE_COMMAND} -E echo "Report: file://${CMAKE_BINARY_DIR}/coverage/html/index.html"
            COMMAND xdg-open ${CMAKE_BINARY_DIR}/coverage/html/index.html || true
            DEPENDS coverage-report
            COMMENT "Opening coverage report"
        )
        
        # Clean coverage
        add_custom_target(coverage-clean
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage
            COMMENT "Cleaning coverage data"
        )
        
        # All coverage targets
        add_custom_target(coverage
            DEPENDS coverage-view
            COMMENT "Running coverage analysis"
        )
    else()
        message(WARNING "Coverage tools not found: gcov=${GCOV_TOOL}, lcov=${LCOV_TOOL}, genhtml=${GENHTML_TOOL}")
    endif()
endfunction()