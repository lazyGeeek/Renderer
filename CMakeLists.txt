set(RENDERER_PROJECT "Renderer")

find_package(Vulkan REQUIRED)

file(
    GLOB_RECURSE RENDERER_CODE_LIST
    LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/*.h*"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/*.inl"
)

add_library(${RENDERER_PROJECT} SHARED ${RENDERER_CODE_LIST})

target_compile_features(${RENDERER_PROJECT} PRIVATE cxx_std_20)
target_include_directories(${RENDERER_PROJECT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include
    ${CMAKE_SOURCE_DIR}/Window/include
    ${CMAKE_SOURCE_DIR}/Window/thirdparty/include
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${RENDERER_PROJECT} PRIVATE ${Vulkan_LIBRARIES} Window)

foreach(RENDERER_FILE IN ITEMS ${RENDERER_CODE_LIST})
    get_filename_component(RENDERER_FILE_PATH "${RENDERER_FILE}" PATH)
    file(RELATIVE_PATH RENDERER_FILE_RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" "${RENDERER_FILE_PATH}")
    string(REPLACE "\\" "/" GROUP_PATH "${RENDERER_FILE_RELATIVE_PATH}")
    file(RELATIVE_PATH EXTERNAL_INCLUDE_RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include")
    string(REPLACE ${EXTERNAL_INCLUDE_RELATIVE_PATH} "" GROUP_PATH "${GROUP_PATH}")
    string(REPLACE "src" "" GROUP_PATH "${GROUP_PATH}")
    string(REPLACE "include/renderer" "" GROUP_PATH "${GROUP_PATH}")
    source_group("${GROUP_PATH}" FILES "${RENDERER_FILE}")
endforeach()
