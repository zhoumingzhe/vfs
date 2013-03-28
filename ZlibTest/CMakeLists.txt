
AUX_SOURCE_DIRECTORY(. zlibtest)
add_library(zlibtest ${zlibtest})
set_property(TARGET zlibtest PROPERTY FOLDER test)