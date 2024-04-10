
# add the executable
add_library(TelnetServerLib STATIC
    telnetservlib.cpp
    telnetsession.cpp
)

target_include_directories(TelnetServerLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
