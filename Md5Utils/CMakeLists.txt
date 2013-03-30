
AUX_SOURCE_DIRECTORY(. Md5Utils)
add_library(Md5Utils ${Md5Utils})
target_link_libraries(Md5Utils vfs)