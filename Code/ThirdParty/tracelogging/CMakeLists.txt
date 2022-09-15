ez_cmake_init()

ez_requires(EZ_CMAKE_PLATFORM_LINUX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
message(WARNING "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

find_package(LTTngUST REQUIRED)
find_package(lttng-ust-tracepoint REQUIRED)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(STATIC_LIBRARY ${PROJECT_NAME} NO_EZ_PREFIX)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC LTTng::UST PRIVATE lttng-ust-tracepoint::lttng-ust-tracepoint)
