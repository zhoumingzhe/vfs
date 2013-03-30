
AUX_SOURCE_DIRECTORY(. DiskFileRWSTest)
add_executable(DiskFileRWSTest ${DiskFileRWSTest})
target_link_libraries(DiskFileRWSTest vfs)
set_property(TARGET DiskFileRWSTest PROPERTY FOLDER test)