# Retrieve the absolute path to qmake and then use that path to find
# the windeployqt executable

find_package(Qt6 REQUIRED COMPONENTS Network)
if (${Qt6_FOUND})
    message("Found Qt " ${Qt6_VERSION}) 
else()
    message("Couldn't find Qt6")
endif()

get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

#find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
#find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

function(windeployqt target)

    # POST_BUILD step
    # - after build, we have a bin/lib for analyzing qt dependencies
    # - we run windeployqt on target and deploy Qt libs
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe"         
                --verbose 1
                --no-svg
                --no-opengl
                --no-opengl-sw
                --no-compiler-runtime
                --no-system-d3d-compiler
                \"$<TARGET_FILE:${target}>\"
        COMMENT "Deploying Qt libraries using windeployqt for compilation target '${target}' ..."
    )

endfunction()