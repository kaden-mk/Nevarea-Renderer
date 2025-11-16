add_executable(EditorApp
    src/main.cpp
)

target_link_libraries(EditorApp PRIVATE NevareaRenderer)
target_include_directories(EditorApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../external
)