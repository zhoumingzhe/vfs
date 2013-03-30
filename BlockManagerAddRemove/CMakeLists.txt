
AUX_SOURCE_DIRECTORY(. BlockManagerAddRemove)
add_executable(BlockManagerAddRemove ${BlockManagerAddRemove})
target_link_libraries(BlockManagerAddRemove vfs)
set_property(TARGET BlockManagerAddRemove PROPERTY FOLDER test)