
AUX_SOURCE_DIRECTORY(. BlockFSCreate)
add_executable(BlockFSCreate ${BlockFSCreate})
target_link_libraries(BlockFSCreate vfs)
set_property(TARGET BlockFSCreate PROPERTY FOLDER test)