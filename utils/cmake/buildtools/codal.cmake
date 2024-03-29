add_executable(
    ${device.device}
    ${SOURCE_FILES}
)

if("${INCLUDE_DIRS}" STRGREATER "")
    target_include_directories(${device.device} PUBLIC "${INCLUDE_DIRS}")
endif()

set_target_properties(${device.device} PROPERTIES SUFFIX "" ENABLE_EXPORTS ON)

# link the executable with supporting libraries.
target_link_libraries(
    ${device.device}
    ${CODAL_DEPS}
)

# import toolchain bin generation command
if(${device.generate_bin})
    include(${TOOLCHAIN_FOLDER}/bin-generator.cmake)
endif()

# import toolchain hex generation command
if(${device.generate_hex})
    include(${TOOLCHAIN_FOLDER}/hex-generator.cmake)
endif()

if("${device.pre_process}" STRGREATER "")
    FORM_SHELL_COMMAND(device device.pre_process SHELL_COMMAND)
    add_custom_target(
        pre-process-task
        COMMAND ${SHELL_COMMAND}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Executing pre process command"
    )
    add_dependencies(${device.device} pre-process-task)
endif()

# post process command hook, depends on the hex file generated by the build system.
if("${device.post_process.command}" STRGREATER "" OR "${device.post_process}" STRGREATER "")
    FORM_SHELL_COMMAND(device device.post_process FINAL_COMMAND)
    # execute
    if("${device.post_process.depends}" STREQUAL "ELF")
        add_custom_command(
            TARGET ${device.device}
            COMMAND ${FINAL_COMMAND}
            DEPENDS  ${device.device}
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            COMMENT "Executing post process command"
        )
    elseif("${device.post_process.depends}" STREQUAL "HEX")
        add_custom_command(
            TARGET ${device.device}_hex
            COMMAND ${FINAL_COMMAND}
            DEPENDS  ${device.device}
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            COMMENT "Executing post process command"
        )
    else()
        #by default post process should depend on hex
        add_custom_command(
            TARGET ${device.device}_bin
            COMMAND ${FINAL_COMMAND}
            DEPENDS  ${device.device}
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            COMMENT "Executing post process command"
        )
    endif()

endif()