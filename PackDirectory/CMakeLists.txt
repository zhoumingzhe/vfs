
AUX_SOURCE_DIRECTORY(. PackDirectory)
add_executable(PackDirectory ${PackDirectory})
target_link_libraries(PackDirectory vfs)
#set_property(TARGET PackDirectory PROPERTY FOLDER PackDirectory)