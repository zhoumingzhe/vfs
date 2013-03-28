
AUX_SOURCE_DIRECTORY(. BlockFSCreate)
add_library(BlockFSCreate ${BlockFSCreate})
set_property(TARGET BlockFSCreate PROPERTY FOLDER test)