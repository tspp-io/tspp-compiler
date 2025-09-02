add_library(tspp_std STATIC
    console.cpp
    errors.cpp
    fmt.cpp
    fs.cpp
    go_builtins.cpp
    io.cpp
    os.cpp
    path.cpp
    std_entrypoint.cpp
    strconv.cpp
    string_util.cpp
    strings.cpp
)

target_include_directories(tspp_std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tspp_std PUBLIC stdc++fs)
