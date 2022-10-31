file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)

add_library(common STATIC)
target_sources(common PUBLIC ${sources})
target_include_directories(common PUBLIC .)
set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)