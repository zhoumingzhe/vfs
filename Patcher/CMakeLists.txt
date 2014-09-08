
AUX_SOURCE_DIRECTORY(. Patcher)
add_executable(Patcher ${Patcher})
target_link_libraries(Patcher vfs Md5Utils)
#set_property(TARGET Patcher PROPERTY FOLDER Patcher)