
AUX_SOURCE_DIRECTORY(. DiskFileCreateTest)
add_executable(DiskFileCreateTest ${DiskFileCreateTest})
target_link_libraries(DiskFileCreateTest vfs)
set_property(TARGET DiskFileCreateTest PROPERTY FOLDER test)