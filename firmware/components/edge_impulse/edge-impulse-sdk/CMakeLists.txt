# components/edge_impulse/CMakeLists.txt

# Recursively find all C and C++ files in the Edge Impulse folders
file(GLOB_RECURSE SOURCES 
    "edge-impulse-sdk/*.cpp" 
    "edge-impulse-sdk/*.c" 
    "edge-impulse-sdk/*.cc"
    "tflite-model/*.cpp"
)

# Register the component with ESP-IDF
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "edge-impulse-sdk" "model-parameters" "tflite-model"
    REQUIRES stdc++
)

# Apply compilation flags required by Edge Impulse and TFLite
target_compile_options(${COMPONENT_LIB} PRIVATE -O3 -g3 -Wno-strict-aliasing)