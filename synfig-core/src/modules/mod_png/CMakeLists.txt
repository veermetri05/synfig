add_library(mod_png MODULE
        "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mptr_png.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_png_spritesheet.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trgt_png.cpp"
)

target_link_libraries(mod_png libsynfig PkgConfig::LIBPNG)

install (
    TARGETS mod_png
    DESTINATION lib/synfig/modules
)
