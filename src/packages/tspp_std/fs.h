#pragma once
#ifdef __cplusplus
extern "C" {
#endif

const char* tspp_fs_readFile(const char* path);
void tspp_fs_writeFile(const char* path, const char* content);
bool tspp_fs_exists(const char* path);

#ifdef __cplusplus
}
#endif
