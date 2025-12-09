// C ABI declarations for tspp runtime syscall wrappers
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Returns a pointer to a (possibly static) C string representation.
// Many bindings currently return `const char*` to point into a static
// buffer; callers should *not* free these. `tspp_free_string` is provided
// for future ownership models and currently is a no-op.
const char* tspp_sys_get_platform();

// Execute a system command. Current implementation returns captured stdout
// as a C string (const char*). Caller must not free the returned pointer.
const char* tspp_sys_system_call(const char* cmd);

// Read entire file; returns pointer to data (const char*) or NULL on
// failure. Caller must not free the returned pointer.
const char* tspp_sys_read_file(const char* path);

// Write buffer to file; returns 0 on success, non-zero on error.
int tspp_sys_write_file(const char* path, const char* buf, int append);

// Environment helpers
const char* tspp_sys_get_env(const char* name);
int tspp_sys_set_env(const char* name, const char* value);

// Free a string allocated by the runtime. Current implementation accepts
// void* and is a no-op; signature kept generic for future changes.
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
