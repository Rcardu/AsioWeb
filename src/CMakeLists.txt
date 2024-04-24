set(LIBRARY_OUTPUT_PATH ${LPATH})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${HPATH})

aux_source_directory(./ SRC)
link_directories(${Boost_LIBRARY_DIR})
add_library(${BSIOLIB} STATIC ${SRC})
target_link_libraries(${BSIOLIB} ${Boost_LIBS})
