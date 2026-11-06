
#
# JavaScript/WebAssembly module (Emscripten + embind)
#

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "OPTION_BUILD_WASM requires an Emscripten toolchain (configure with emcmake cmake ...).")
endif()

# Target name
set(target girgs-wasm)
message(STATUS "Wasm ${target}")


#
# Create module
#

add_executable(${target} bindings.cpp)

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
    CXX_STANDARD 17
    OUTPUT_NAME "girgs"
    SUFFIX ".mjs"
)

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    girgs::girgs
    girgs::hypergirgs
)

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)

target_link_options(${target}
    PRIVATE
    -lembind
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sEXPORT_NAME=createGirgs
    -sENVIRONMENT=web,worker,node
    -sALLOW_MEMORY_GROWTH=1
)


#
# Deployment
#

install(FILES
    ${PROJECT_BINARY_DIR}/girgs.mjs
    ${PROJECT_BINARY_DIR}/girgs.wasm
    girgs.d.ts
    package.json
    DESTINATION ${INSTALL_ROOT}/wasm
    COMPONENT runtime
)
