#pragma once

#include <string>
#include <vector>

namespace tspp::core::syscall {

/**
 * @brief OS-specific system call wrapper
 * Provides a unified interface for making system calls across different
 * platforms. This allows TSPP programs to directly invoke system calls without
 * C++ wrapper libraries.
 */

// System Information
std::string get_platform();  // Returns: "linux", "darwin", "win32", "unknown"
int get_cpu_count();         // Returns number of CPUs
std::string get_cwd();       // Returns current working directory
int set_cwd(const std::string&
                path);  // Changes working directory, returns 0 on success

// Process Management
std::string system_call(
    const std::string& command);  // Execute a shell command and return stdout
int get_pid();                    // Get process ID
int exit_process(int code);       // Exit process

// File System Operations
bool file_exists(const std::string& path);
std::string read_file(const std::string& path);
int write_file(const std::string& path, const std::string& content,
               bool append = false);
int delete_file(const std::string& path);
bool is_directory(const std::string& path);
std::vector<std::string> list_directory(const std::string& path);

// Path Utilities
std::string path_join(const std::string& a, const std::string& b);
std::string path_basename(const std::string& path);
std::string path_dirname(const std::string& path);
std::string path_absolute(const std::string& path);

// Environment Variables
std::string get_env(const std::string& name);
int set_env(const std::string& name, const std::string& value);

// ============================================================================
// Network
// ============================================================================

// Perform an HTTP GET and return the response body (empty string on error)
std::string http_get(const std::string& url);

// Basic TCP socket helpers: return a socket handle (int) or -1 on error
int tcp_connect(const std::string& host, int port);
int tcp_send(int sockfd, const std::string& data);
int tcp_close(int sockfd);
// Start a simple background TCP server that listens on localhost:port and
// echoes received data back to the client. Returns 0 on success, -1 on error.
int tcp_server_start(int port);
// Stop a previously started server listening on `port`. Returns 0 on success,
// -1 if no server was found or on error.
int tcp_server_stop(int port);
// Accept a pending client connection for server listening on `port`.
// Returns a client socket fd (>0) or -1 if no pending client.
// Accept a pending client connection for server listening on `port`.
// Blocks until a client is available or until `timeout_ms` milliseconds elapse.
// If `timeout_ms` <= 0, blocks indefinitely. Returns a client socket fd (>0) or
// -1 on error/timeout.
int tcp_server_accept(int port, int timeout_ms);

// Receive up to `maxlen` bytes from socket `sockfd`. Returns received data
// as a string (empty string on EOF or error).
std::string tcp_recv(int sockfd, int maxlen);
// Receive into provided buffer `buf` up to `maxlen` bytes. Returns number
// of bytes read (0 on EOF, -1 on error).
int tcp_recv_into(int sockfd, char* buf, int maxlen);

// Simple malloc/free wrappers exposed for TSPP stdlib use
void* ts_malloc(int size);
void ts_free(void* p);

// Construct a string from a raw buffer and length (returns runtime-managed
// std::string c_str)
std::string string_from_buffer(const char* buf, int len);

// ============================================================================
// Graphics (simple canvas backed by in-memory buffers saved as PPM)
// ============================================================================

// Create a simple RGB canvas and return a handle (>0) or -1 on error
int create_canvas(int width, int height);
// Draw a filled rectangle on the canvas (r,g,b in 0-255)
int draw_rect(int canvasHandle, int x, int y, int w, int h, int r, int g,
              int b);
// Present (write) the canvas to a PPM file and return the path
std::string present_canvas(int canvasHandle, const std::string& path);

}  // namespace tspp::core::syscall
