
AUX_SOURCE_DIRECTORY(. BlockFSPackDirectory)
add_executable(BlockFSPackDirectory ${BlockFSPackDirectory})
target_link_libraries(BlockFSPackDirectory vfs)
set_property(TARGET BlockFSPackDirectory PROPERTY FOLDER test)