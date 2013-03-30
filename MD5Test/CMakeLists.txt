
AUX_SOURCE_DIRECTORY(. MD5Test)
add_executable(MD5Test ${MD5Test})
target_link_libraries(MD5Test vfs)
set_property(TARGET MD5Test PROPERTY FOLDER test)