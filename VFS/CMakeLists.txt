
AUX_SOURCE_DIRECTORY(. VFS)
add_library(vfs ${VFS})
target_link_libraries(vfs zlib Shlwapi)
#set_property(TARGET vfs PROPERTY FOLDER vfs)