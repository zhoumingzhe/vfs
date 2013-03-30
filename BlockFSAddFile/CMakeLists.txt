
AUX_SOURCE_DIRECTORY(. BlockFSAddFile)
add_executable(BlockFSAddFile ${BlockFSAddFile})
target_link_libraries(BlockFSAddFile vfs)
set_property(TARGET BlockFSAddFile PROPERTY FOLDER test)