
set(_TARGET_NAME "002-vulkan")
set(_TARGET_DIR "${_SOURCE_DIR}/${_TARGET_NAME}")

message("============================================================================")
message("_TARGET_NAME = ${_TARGET_NAME}")
message("_TARGET_DIR = ${_TARGET_DIR}")

set(_HEADER_DIRS
        "$ENV{VULKAN_SDK_PATH}/macOS/include"
        "$ENV{GLFW_SDK_PATH}/include"
        "${_SOURCE_DIR}"
)

set(_LIBRARY_DIRS
        "$ENV{VULKAN_SDK_PATH}/macOS/lib"
        "$ENV{GLFW_SDK_PATH}/lib-x86_64"
)


file(GLOB _HEADER_FILES
        "${_TARGET_DIR}/*.h"
)

file(GLOB _SOURCE_FILES
        "${_TARGET_DIR}/*.cc"
        "${_TARGET_DIR}/${_OS_NAME}/*.cc"
)

set(_LIBRARY_FILES
        "glfw3"
        "libvulkan.dylib"
        "libvulkan.1.dylib"
        "libvulkan.1.3.280.dylib"
)

message("_HEADER_DIRS = ${_HEADER_DIRS}")
message("_LIBRARY_DIRS = ${_LIBRARY_DIRS}")
message("_HEADER_FILES = ${_HEADER_FILES}")
message("_SOURCE_FILES = ${_SOURCE_FILES}")
message("_LIBRARY_FILES = ${_LIBRARY_FILES}")

add_executable(${_TARGET_NAME} ${_HEADER_FILES} ${_SOURCE_FILES})
target_include_directories(${_TARGET_NAME} PRIVATE ${_HEADER_DIRS})
target_link_directories(${_TARGET_NAME} PRIVATE ${_LIBRARY_DIRS})
target_link_libraries(${_TARGET_NAME} ${_LIBRARY_FILES})


install(FILES ${_HEADER_FILES} DESTINATION include/${TARGET_NAME})
install(TARGETS ${_TARGET_NAME} DESTINATION lib/${_OS_NAME}/${CMAKE_BUILD_TYPE})

