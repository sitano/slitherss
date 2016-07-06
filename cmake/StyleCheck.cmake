find_package(PythonInterp)

# set(STYLE_FILTER)

# Disable 80 columns per line
# set (STYLE_FILTER ${STYLE_FILTER},-whitespace/line_length)

set(STYLE_CHECK_DIR "${CMAKE_SOURCE_DIR}/third_party/cpplint")
set(STYLE_CHECK_SCRIPT "${STYLE_CHECK_DIR}/cpplint.py")

# Add a target that runs cpplint.py
#
# Parameters:
# - SOURCES_LIST a complete list of source and include files to check
function(add_style_check_target PROJECT SOURCES_LIST)

    if(NOT PYTHONINTERP_FOUND)
        return()
    endif()

    list(SORT SOURCES_LIST)
    list(REMOVE_DUPLICATES SOURCES_LIST)

    add_custom_target("${PROJECT}_style"
        COMMAND "${CMAKE_COMMAND}" -E chdir
            "${CMAKE_SOURCE_DIR}"
            "${PYTHON_EXECUTABLE}"
            "${STYLE_CHECK_SCRIPT}"
            # "--filter=${STYLE_FILTER}"
            ${SOURCES_LIST}
        DEPENDS ${SOURCES_LIST} ${STYLE_CHECK_SCRIPT}
        COMMENT "Checking code style."
        VERBATIM
    )

    add_dependencies(${PROJECT} ${PROJECT}_style)

endfunction(add_style_check_target)
