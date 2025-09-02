add_library(tspp_std STATIC
    console.cpp
    fs.cpp
    go_builtins.cpp
    os.cpp
    path.cpp
    std_entrypoint.cpp
    string_util.cpp
)

target_include_directories(tspp_std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tspp_std PUBLIC stdc++fs)
