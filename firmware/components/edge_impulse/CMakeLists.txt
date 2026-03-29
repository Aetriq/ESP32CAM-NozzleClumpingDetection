# components/edge_impulse/CMakeLists.txt

# 1. Find ALL source files
file(GLOB_RECURSE ALL_SOURCES 
    "edge-impulse-sdk/*.cpp" 
    "edge-impulse-sdk/*.c" 
    "edge-impulse-sdk/*.cc"
    "tflite-model/*.cpp"
    "tflite-model/*.c"
)

# 2. Filter out unwanted platform ports (CRITICAL FIX)
set(SOURCES "")
foreach(src_file ${ALL_SOURCES})
    # If the file is in a porting folder, BUT it is not the espressif folder, skip it.
    if(src_file MATCHES "porting/" AND NOT src_file MATCHES "porting/espressif/")
        # Do nothing (skip)
    else()
        list(APPEND SOURCES ${src_file})
    endif()
endforeach()

# 3. Register the component and link esp_timer
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "edge-impulse-sdk" "model-parameters" "tflite-model"
    REQUIRES esp-dsp esp-nn esp_timer
)

# 4. Apply compilation flags
target_compile_options(${COMPONENT_LIB} PRIVATE -O3 -g3 -Wno-strict-aliasing -Wno-maybe-uninitialized -Wno-missing-field-initializers -Wno-error)