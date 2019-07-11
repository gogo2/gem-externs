project(gem_externs_main)

set(SRC_FILES
        pix_multiblob2.cpp
        gem_utils/plist.cpp
        gem_utils/pstk.cpp
        )


add_library(pix_multiblob2 SHARED ${SRC_FILES})

set_target_properties(pix_multiblob2 PROPERTIES PREFIX "")
if (WIN32)
    set_target_properties(pix_multiblob2 PROPERTIES SUFFIX ".dll")
elseif (APPLE)
    set_target_properties(pix_multiblob2 PROPERTIES SUFFIX ".pd_darwin")
elseif (UNIx)
    set_target_properties(pix_multiblob2 PROPERTIES SUFFIX ".pd_linux")
endif ()

target_link_libraries(pix_multiblob2 PRIVATE
        ${PD_LIB_NAME}
        ${GEM_LIB_NAME}
        )
