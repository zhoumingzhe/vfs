
AUX_SOURCE_DIRECTORY(. BlockFSRemove)
add_executable(BlockFSRemove ${BlockFSRemove})
target_link_libraries(BlockFSRemove vfs)
set_property(TARGET BlockFSRemove PROPERTY FOLDER test)