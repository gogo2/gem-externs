project(gem_externs_main)

set(SRC_FILES
        pix_multiblob2.cpp
        gem_utils/plist.cpp
        gem_utils/pstk.cpp
        )

add_library(pix_multiblob2 SHARED ${SRC_FILES})

find_package(PD REQUIRED)
find_package(Gem REQUIRED)

target_compile_options(pix_multiblob2 PRIVATE $<$<CXX_COMPILER_ID:GNU>:-O3 -pedantic -Wall -static-libstdc++>)

set_target_properties(pix_multiblob2 PROPERTIES CXX_STANDARD 11)

target_include_directories(pix_multiblob2 PRIVATE
        ${PD_INCLUDE_DIRS}
        ${GEM_INCLUDE_DIRS}
        )

target_link_libraries(pix_multiblob2 PRIVATE
        ${PD_LIBRARIES}
        ${GEM_LIBRARIES}
        )
