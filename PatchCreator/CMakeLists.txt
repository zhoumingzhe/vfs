
AUX_SOURCE_DIRECTORY(. PatchCreator)
add_executable(PatchCreator ${PatchCreator})
target_link_libraries(PatchCreator vfs Md5Utils)
#set_property(TARGET PatchCreator PROPERTY FOLDER PatchCreator)