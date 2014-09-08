
AUX_SOURCE_DIRECTORY(. PatchLib)
add_library(PatchLib ${PatchLib})
target_link_libraries(PatchLib vfs Md5Utils)
#set_property(TARGET PatchLib PROPERTY FOLDER PatchLib)
