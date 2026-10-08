// C ABI declarations for tspp runtime syscall wrappers
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Ownership: every `const char*` returned by the runtime is a fresh,
// NUL-terminated buffer owned by the Boehm GC. Callers never free it; it stays
// valid while referenced. `tspp_free_string` is a no-op kept for ABI
// compatibility.

// Runtime initialization (Boehm GC). The generated `main` calls it first.
void tspp_runtime_init(void);

const char* tspp_sys_get_platform();

// Execute a system command; returns captured stdout (GC-owned).
const char* tspp_sys_system_call(const char* cmd);

// Read an entire file on every call; returns its contents (GC-owned), or ""
// on failure.
const char* tspp_sys_read_file(const char* path);

// Write buffer to file; returns 0 on success, non-zero on error.
int tspp_sys_write_file(const char* path, const char* buf, int append);

// Environment helpers
const char* tspp_sys_get_env(const char* name);
int tspp_sys_set_env(const char* name, const char* value);

// No-op: runtime strings are GC-owned. Kept for ABI compatibility.
void tspp_free_string(void* s);

// Additional helpers provided by the runtime (string conversions,
// concatenation, console) used by codegen.
const char* tspp_string_concat(const char* a, const char* b);
const char* tspp_int_to_string(int val);
const char* tspp_bool_to_string(int val);
const char* tspp_float_to_string(double val);
const char* tspp_ptr_to_string(void* ptr);
void tspp_console_log(const char* msg);
void tspp_console_error(const char* msg);
void tspp_console_warn(const char* msg);

#ifdef __cplusplus
}
#endif
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Runtime lifecycle
// ============================================================================

void tspp_runtime_init(void);

// ============================================================================
// System Information
// ============================================================================

const char* tspp_sys_get_platform();
int tspp_sys_get_cpu_count();
const char* tspp_sys_get_cwd();
int tspp_sys_set_cwd(const char* path);

// ============================================================================
// Process Management
// ============================================================================

const char* tspp_sys_system_call(const char* command);
int tspp_sys_get_pid();
void tspp_sys_exit_process(int code);

// ============================================================================
// File System Operations
// ============================================================================

int tspp_sys_file_exists(const char* path);
const char* tspp_sys_read_file(const char* path);
int tspp_sys_write_file(const char* path, const char* content, int append);
int tspp_sys_delete_file(const char* path);
int tspp_sys_is_directory(const char* path);

// ============================================================================
// String Utilities (helper for codegen)
// ============================================================================

const char* tspp_string_concat(const char* a, const char* b);
const char* tspp_int_to_string(int val);
const char* tspp_bool_to_string(int val);
const char* tspp_float_to_string(double val);
const char* tspp_ptr_to_string(void* ptr);
void tspp_console_log(const char* msg);
void tspp_console_error(const char* msg);
void tspp_console_warn(const char* msg);
void tspp_free_string(void* p);

// ============================================================================
// Network
// ============================================================================

const char* tspp_sys_http_get(const char* url);
int tspp_sys_tcp_connect(const char* host, int port);
int tspp_sys_tcp_send(int sockfd, const char* buf, int len);
int tspp_sys_tcp_close(int sockfd);
int tspp_sys_tcp_server_start(int port);
int tspp_sys_tcp_server_stop(int port);
int tspp_sys_tcp_server_accept(int port, int timeout_ms);
const char* tspp_sys_tcp_recv(int sockfd, int maxlen);
int tspp_sys_tcp_recv_into(int sockfd, char* buf, int maxlen);
void* tspp_sys_malloc(int size);
void tspp_sys_free(void* p);
const char* tspp_sys_string_from_buffer(const char* buf, int len);

// ============================================================================
// Graphics
// ============================================================================

int tspp_sys_create_canvas(int width, int height);
int tspp_sys_draw_rect(int canvasHandle, int x, int y, int w, int h, int r,
                       int g, int b);
const char* tspp_sys_present_canvas(int canvasHandle, const char* path);

#ifdef __cplusplus
}
#endif
