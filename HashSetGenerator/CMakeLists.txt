
AUX_SOURCE_DIRECTORY(. HashSetGenerator)
add_executable(HashSetGenerator ${HashSetGenerator})
target_link_libraries(HashSetGenerator vfs)
#set_property(TARGET HashSetGenerator PROPERTY FOLDER HashSetGenerator)