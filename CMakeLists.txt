add_library(dppinteract)

set_target_properties(dppinteract
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

option(USE_COROUTINES "Use DPP coroutine support. Needed for coroutine next_* functions." OFF)
if(USE_COROUTINES)
    target_compile_definitions(dppinteract PUBLIC DPP_CORO)
endif()

target_sources(dppinteract
    PRIVATE
    dppinteract/interactiveservice.cpp
    dppinteract/pagination/interactionpage.cpp
    dppinteract/pagination/paginator.cpp
    dppinteract/pagination/staticpaginator.cpp
    PUBLIC
        FILE_SET HEADERS FILES
        dppinteract/entityfilterdata.h
        dppinteract/interactivedata.h
        dppinteract/interactiveresult.h
        dppinteract/interactiveservice.h
        dppinteract/pagination/interactionpage.h
        dppinteract/pagination/paginator.h
        dppinteract/pagination/staticpaginator.h)

find_package(DPP REQUIRED)
target_link_libraries(dppinteract PUBLIC dpp::dpp)
