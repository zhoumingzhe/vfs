
AUX_SOURCE_DIRECTORY(. DiskFileReadTest)
add_executable(DiskFileReadTest ${DiskFileReadTest})
target_link_libraries(DiskFileReadTest vfs)
set_property(TARGET DiskFileReadTest PROPERTY FOLDER test)