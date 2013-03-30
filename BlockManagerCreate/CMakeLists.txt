
AUX_SOURCE_DIRECTORY(. BlockManagerCreate)
add_executable(BlockManagerCreate ${BlockManagerCreate})
target_link_libraries(BlockManagerCreate vfs)
set_property(TARGET BlockManagerCreate PROPERTY FOLDER test)