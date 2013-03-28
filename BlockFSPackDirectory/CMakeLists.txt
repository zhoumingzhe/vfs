
AUX_SOURCE_DIRECTORY(. BlockFSPackDirectory)
add_library(BlockFSPackDirectory ${BlockFSPackDirectory})
set_property(TARGET BlockFSPackDirectory PROPERTY FOLDER test)