add_library(dpp-interactive)

set_target_properties(dpp-interactive
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

option(USE_COROUTINES "Use DPP coroutine support." ON)

if(USE_COROUTINES)
    target_compile_definitions(dpp-interactive PUBLIC DPP_CORO)
endif()

target_sources(dpp-interactive
    PRIVATE
        dpp-interactive/interactivedata.cpp
        dpp-interactive/interactiveservice.cpp
        dpp-interactive/pagination/interactionpage.cpp
        dpp-interactive/pagination/paginator.cpp
        dpp-interactive/pagination/staticpaginator.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            dpp-interactive/interactivedata.h
            dpp-interactive/interactiveservice.h
            dpp-interactive/pagination/interactionpage.h
            dpp-interactive/pagination/paginator.h
            dpp-interactive/pagination/staticpaginator.h
            dpp-interactive/utils/ezcoro.h)

find_package(DPP REQUIRED)
target_link_libraries(dpp-interactive PUBLIC dpp::dpp)
