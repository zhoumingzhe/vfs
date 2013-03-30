
AUX_SOURCE_DIRECTORY(. zlibtest)
add_executable(zlibtest ${zlibtest})
target_link_libraries(zlibtest zlib)
set_property(TARGET zlibtest PROPERTY FOLDER test)