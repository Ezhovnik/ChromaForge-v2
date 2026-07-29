set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/AppDir/usr)

install(TARGETS ChromaForge DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/res/ DESTINATION share/ChromaForge/res)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/AppDir/usr/share/icons/hicolor/256x256)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/AppDir/usr/share/applications)

configure_file(${CMAKE_SOURCE_DIR}/dev/ChromaForge.desktop
               ${CMAKE_BINARY_DIR}/AppDir/usr/share/applications/ChromaForge.desktop)

file(COPY ${CMAKE_SOURCE_DIR}/dev/ChromaForge.png
     DESTINATION ${CMAKE_BINARY_DIR}/AppDir/usr/share/icons/hicolor/256x256/)
