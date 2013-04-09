find_package(qt4 REQUIRED QtCore QtNetwork)
include(${QT_USE_FILE})
AUX_SOURCE_DIRECTORY(. Downloader)
QT4_WRAP_CPP(Downloader_H downloadmanager.h hashchecker.h updatemanager.h)
add_executable(Downloader ${Downloader} ${Downloader_H})
target_link_libraries(Downloader vfs Md5Utils)

QT4_USE_MODULES(Downloader Core Network)