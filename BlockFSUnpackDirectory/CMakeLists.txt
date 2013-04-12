
AUX_SOURCE_DIRECTORY(. UnpackDirectory)
add_executable(UnpackDirectory ${UnpackDirectory})
target_link_libraries(UnpackDirectory vfs)
#set_property(TARGET UnpackDirectory PROPERTY FOLDER UnpackDirectory)