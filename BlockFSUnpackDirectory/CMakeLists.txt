
AUX_SOURCE_DIRECTORY(. BlockFSUnpackDirectory)
add_executable(BlockFSUnpackDirectory ${BlockFSUnpackDirectory})
target_link_libraries(BlockFSUnpackDirectory vfs)
set_property(TARGET BlockFSUnpackDirectory PROPERTY FOLDER test)