
AUX_SOURCE_DIRECTORY(. BlockFSUnpackDirectory)
add_library(BlockFSUnpackDirectory ${BlockFSUnpackDirectory})
set_property(TARGET BlockFSUnpackDirectory PROPERTY FOLDER test)