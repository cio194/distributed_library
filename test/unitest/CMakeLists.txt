project(unitest)

FILE(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS *.cpp)

FOREACH (F ${ALL_SRC})
    get_filename_component(TName ${F} NAME_WE)
    add_executable(${TName} ${F})
    target_link_libraries(${TName} PUBLIC logic_server_static test_utils)
ENDFOREACH (F)