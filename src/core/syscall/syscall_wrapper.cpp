#include "syscall_wrapper.h"

#include <curl/curl.h>
#include <gc.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cerrno>
namespace tspp::core::syscall {

// ============================================================================
// System Information
// ============================================================================

std::string get_platform() {
#if defined(_WIN32)
  return "win32";
#elif defined(__linux__)
  return "linux";
#elif defined(__APPLE__)
  return "darwin";
#else
  return "unknown";
#endif
}

int get_cpu_count() {
  return static_cast<int>(std::thread::hardware_concurrency());
}

std::string get_cwd() {
  char cwd[4096];
#ifdef _WIN32
  if (_getcwd(cwd, sizeof(cwd)) == nullptr) {
    return "";
  }
#else
  if (getcwd(cwd, sizeof(cwd)) == nullptr) {
    return "";
  }
#endif
  return std::string(cwd);
}

int set_cwd(const std::string& path) {
#ifdef _WIN32
  return _chdir(path.c_str()) == 0 ? 0 : -1;
#else
  return chdir(path.c_str()) == 0 ? 0 : -1;
#endif
}

// ============================================================================
// Process Management
// ============================================================================

std::string system_call(const std::string& command) {
#ifdef _WIN32
  FILE* pipe = _popen(command.c_str(), "r");
#else
  FILE* pipe = popen(command.c_str(), "r");
#endif
  if (!pipe) return std::string("");
  std::string output;
  char buf[4096];
  while (fgets(buf, sizeof(buf), pipe) != nullptr) {
    output += buf;
  }
#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return output;
}

int get_pid() {
#ifdef _WIN32
  return static_cast<int>(GetCurrentProcessId());
#else
  return static_cast<int>(getpid());
#endif
}

int exit_process(int code) {
  std::exit(code);
  return 0;  // Never reached
}

// ============================================================================
// File System Operations
// ============================================================================

bool file_exists(const std::string& path) {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

std::string read_file(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int write_file(const std::string& path, const std::string& content,
               bool append) {
  std::ios_base::openmode mode = std::ios::binary;
  if (append) {
    mode |= std::ios::app;
  }

  std::ofstream file(path, mode);
  if (!file) {
    return -1;
  }

  file.write(content.c_str(), content.size());
  return file.good() ? 0 : -1;
}

int delete_file(const std::string& path) {
  return std::remove(path.c_str()) == 0 ? 0 : -1;
}

bool is_directory(const std::string& path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) != 0) {
    return false;
  }
#ifdef _WIN32
  return (buffer.st_mode & _S_IFDIR) != 0;
#else
  return S_ISDIR(buffer.st_mode);
#endif
}

std::vector<std::string> list_directory(const std::string& path) {
  std::vector<std::string> entries;

#ifdef _WIN32
  WIN32_FIND_DATAA findData;
  HANDLE findHandle = FindFirstFileA((path + "\\*").c_str(), &findData);

  if (findHandle == INVALID_HANDLE_VALUE) {
    return entries;
  }

  do {
    entries.emplace_back(findData.cFileName);
  } while (FindNextFileA(findHandle, &findData));

  FindClose(findHandle);
#else
  DIR* dir = opendir(path.c_str());
  if (!dir) {
    return entries;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    entries.emplace_back(entry->d_name);
  }

  closedir(dir);
#endif

  return entries;
}

// ============================================================================
// Path Utilities
// ============================================================================

std::string path_join(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;

  char separator = '/';
#ifdef _WIN32
  separator = '\\';
#endif

  std::string result = a;
  if (result.back() != separator && result.back() != '/') {
    result += separator;
  }
  result += b;
  return result;
}

std::string path_basename(const std::string& path) {
  size_t lastSlash = path.find_last_of("/\\");
  if (lastSlash != std::string::npos) {
    return path.substr(lastSlash + 1);
  }
  return path;
}

std::string path_dirname(const std::string& path) {
  size_t lastSlash = path.find_last_of("/\\");
  if (lastSlash != std::string::npos) {
    return path.substr(0, lastSlash);
  }
  return ".";
}

std::string path_absolute(const std::string& path) {
  if (path.empty()) return get_cwd();

#ifdef _WIN32
  char fullPath[MAX_PATH];
  if (_fullpath(fullPath, path.c_str(), MAX_PATH) == nullptr) {
    return "";
  }
  return fullPath;
#else
  char fullPath[4096];
  if (realpath(path.c_str(), fullPath) == nullptr) {
    return "";
  }
  return fullPath;
#endif
}

// ============================================================================
// Environment Variables
// ============================================================================

std::string get_env(const std::string& name) {
  const char* value = std::getenv(name.c_str());
  return value ? std::string(value) : "";
}

int set_env(const std::string& name, const std::string& value) {
#ifdef _WIN32
  return _putenv_s(name.c_str(), value.c_str()) == 0 ? 0 : -1;
#else
  return setenv(name.c_str(), value.c_str(), 1) == 0 ? 0 : -1;
#endif
}

// ============================================================================
// Network
// ============================================================================

static size_t curl_write_cb(void* contents, size_t size, size_t nmemb,
                            void* userp) {
  size_t realsize = size * nmemb;
  std::string* mem = reinterpret_cast<std::string*>(userp);
  mem->append(reinterpret_cast<char*>(contents), realsize);
  return realsize;
}

std::string http_get(const std::string& url) {
  CURL* curl = curl_easy_init();
  if (!curl) return std::string("");
  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  if (res != CURLE_OK) return std::string("");
  return response;
}

int tcp_connect(const std::string& host, int port) {
#ifdef _WIN32
  (void)host;
  (void)port;
  return -1;  // not implemented on Windows here
#else
  struct addrinfo hints;
  struct addrinfo* result = nullptr;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  std::string portStr = std::to_string(port);
  if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0)
    return -1;

  int sockfd = -1;
  for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) continue;
    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) break;
    close(sockfd);
    sockfd = -1;
  }
  freeaddrinfo(result);
  return sockfd;
#endif
}

int tcp_send(int sockfd, const std::string& data) {
#ifdef _WIN32
  (void)sockfd;
  (void)data;
  return -1;
#else
  ssize_t total = 0;
  const char* buf = data.c_str();
  size_t remaining = data.size();
  while (remaining > 0) {
    ssize_t sent = send(sockfd, buf + total, remaining, 0);
    if (sent <= 0) {
      int e = errno;
      std::cerr << "tcp_send: send() failed, errno=" << e << " ("
                << std::strerror(e) << ")\n";
      return -1;
    }
    total += sent;
    remaining -= sent;
  }
  return static_cast<int>(total);
#endif
}

int tcp_close(int sockfd) {
#ifdef _WIN32
  (void)sockfd;
  return -1;
#else
  return close(sockfd) == 0 ? 0 : -1;
#endif
}

struct ServerState {
  int server_fd = -1;
  int port = 0;
  std::thread accept_thread;
  std::atomic<bool> running{false};
  std::mutex clients_mutex;
  std::condition_variable clients_cv;
  std::queue<int> pending_clients;
};

// Map server handle -> server state. We return a small integer handle to the
// caller so multiple servers can be managed without relying on port numbers
// as keys.
static std::map<int, std::shared_ptr<ServerState>> g_servers;
static std::mutex g_servers_mutex;
static int g_next_server_handle = 1;

int tcp_server_start(int port) {
#ifdef _WIN32
  (void)port;
  return -1;  // Not implemented on Windows in this runtime
#else
  try {
    // Create, bind and listen synchronously so the caller can connect
    // immediately.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
      std::cerr << "tcp_server_start: socket() failed: " << std::strerror(errno)
                << "\n";
      return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // listen on localhost only
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      std::cerr << "tcp_server_start: bind() failed: " << std::strerror(errno)
                << "\n";
      close(server_fd);
      return -1;
    }

    if (listen(server_fd, 1) < 0) {
      std::cerr << "tcp_server_start: listen() failed: " << std::strerror(errno)
                << "\n";
      close(server_fd);
      return -1;
    }

    // Store server state and spawn an accept loop that handles multiple clients
    auto state = std::make_shared<ServerState>();
    state->server_fd = server_fd;
    state->port = port;
    state->running.store(true);

    state->accept_thread = std::thread([state]() {
      while (state->running.load()) {
        int client = accept(state->server_fd, nullptr, nullptr);
        if (client < 0) {
          if (!state->running.load()) break;  // likely closed during shutdown
          std::cerr << "tcp_server_start: accept() failed: "
                    << std::strerror(errno) << "\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
          continue;
        }

        // Enqueue accepted client for TSPP to accept via tcp_server_accept
        {
          std::lock_guard<std::mutex> lk(state->clients_mutex);
          state->pending_clients.push(client);
        }
        state->clients_cv.notify_one();
      }

      // cleanup on exit
      if (state->server_fd >= 0) {
        close(state->server_fd);
        state->server_fd = -1;
      }
    });
    int handle = 0;
    {
      std::lock_guard<std::mutex> lk(g_servers_mutex);
      handle = g_next_server_handle++;
      g_servers[handle] = state;
    }
    return handle;
  } catch (const std::exception& e) {
    std::cerr << "tcp_server_start: exception while starting thread: "
              << e.what() << "\n";
    return -1;
  }
  return 0;
#endif
}

int tcp_server_stop(int handle) {
#ifdef _WIN32
  (void)port;
  return -1;
#else
  std::shared_ptr<ServerState> s;
  {
    std::lock_guard<std::mutex> lk(g_servers_mutex);
    auto it = g_servers.find(handle);
    if (it == g_servers.end()) return -1;
    s = it->second;
    g_servers.erase(it);
  }

  if (!s) return -1;
  s->running.store(false);
  if (s->server_fd >= 0) {
    // Closing the listening socket will unblock accept
    close(s->server_fd);
    s->server_fd = -1;
  }
  if (s->accept_thread.joinable()) {
    s->accept_thread.join();
  }
  return 0;
#endif
}

#include <chrono>

int tcp_server_accept(int handle, int timeout_ms) {
#ifdef _WIN32
  (void)port;
  (void)timeout_ms;
  return -1;
#else
  std::shared_ptr<ServerState> s;
  {
    std::lock_guard<std::mutex> lk(g_servers_mutex);
    auto it = g_servers.find(handle);
    if (it == g_servers.end()) return -1;
    s = it->second;
  }
  if (!s) return -1;
  // Blocking accept: wait until pending client is available or timeout
  std::unique_lock<std::mutex> lk(s->clients_mutex);
  if (timeout_ms <= 0) {
    s->clients_cv.wait(lk, [&]() {
      return !s->pending_clients.empty() || !s->running.load();
    });
  } else {
    s->clients_cv.wait_for(lk, std::chrono::milliseconds(timeout_ms), [&]() {
      return !s->pending_clients.empty() || !s->running.load();
    });
  }
  if (s->pending_clients.empty()) return -1;
  int client = s->pending_clients.front();
  s->pending_clients.pop();
  return client;
#endif
}

std::string tcp_recv(int sockfd, int maxlen) {
#ifdef _WIN32
  (void)sockfd;
  (void)maxlen;
  return std::string("");
#else
  if (maxlen <= 0) return std::string("");
  std::vector<char> buf((size_t)maxlen);
  ssize_t r = recv(sockfd, buf.data(), (size_t)maxlen, 0);
  if (r <= 0) return std::string("");
  return std::string(buf.data(), (size_t)r);
#endif
}

int tcp_recv_into(int sockfd, char* buf, int maxlen) {
#ifdef _WIN32
  (void)sockfd;
  (void)buf;
  (void)maxlen;
  return -1;
#else
  if (maxlen <= 0 || !buf) return -1;
  ssize_t r = recv(sockfd, buf, (size_t)maxlen, 0);
  if (r < 0) return -1;
  return static_cast<int>(r);
#endif
}

// ============================================================================
// Graphics: simple in-memory canvas saved to PPM
// ============================================================================

struct Canvas {
  int w = 0;
  int h = 0;
  std::vector<uint8_t> pixels;  // RGB triplets
};

static std::map<int, Canvas> g_canvases;
static std::mutex g_canvas_mutex;
static int g_next_canvas_id = 1;

int create_canvas(int width, int height) {
  if (width <= 0 || height <= 0) return -1;
  Canvas c;
  c.w = width;
  c.h = height;
  c.pixels.assign(width * height * 3, 255);
  std::lock_guard<std::mutex> lk(g_canvas_mutex);
  int id = g_next_canvas_id++;
  g_canvases[id] = std::move(c);
  return id;
}

int draw_rect(int canvasHandle, int x, int y, int w, int h, int r, int g,
              int b) {
  std::lock_guard<std::mutex> lk(g_canvas_mutex);
  auto it = g_canvases.find(canvasHandle);
  if (it == g_canvases.end()) return -1;
  Canvas& c = it->second;
  for (int yy = y; yy < y + h; ++yy) {
    if (yy < 0 || yy >= c.h) continue;
    for (int xx = x; xx < x + w; ++xx) {
      if (xx < 0 || xx >= c.w) continue;
      int idx = (yy * c.w + xx) * 3;
      c.pixels[idx + 0] = static_cast<uint8_t>(std::clamp(r, 0, 255));
      c.pixels[idx + 1] = static_cast<uint8_t>(std::clamp(g, 0, 255));
      c.pixels[idx + 2] = static_cast<uint8_t>(std::clamp(b, 0, 255));
    }
  }
  return 0;
}

std::string present_canvas(int canvasHandle, const std::string& path) {
  std::lock_guard<std::mutex> lk(g_canvas_mutex);
  auto it = g_canvases.find(canvasHandle);
  if (it == g_canvases.end()) return std::string("");
  Canvas& c = it->second;
  std::string outPath =
      path.empty()
          ? ("/tmp/tspp_canvas_" + std::to_string(canvasHandle) + ".ppm")
          : path;
  std::ofstream ofs(outPath, std::ios::binary);
  if (!ofs) return std::string("");
  ofs << "P6\n" << c.w << " " << c.h << "\n255\n";
  ofs.write(reinterpret_cast<char*>(c.pixels.data()), c.pixels.size());
  ofs.close();
  return outPath;
}

}  // namespace tspp::core::syscall

// ============================================================================
// C Bindings for LLVM Codegen (extern "C" so TSPP can call them)
// ============================================================================

// ============================================================================
// String ownership
// ============================================================================
//
// Every `const char*` the runtime hands to tspp code is a fresh, NUL-terminated
// copy in memory owned by the Boehm GC (GC_MALLOC_ATOMIC: the GC does not scan
// it for pointers). The collector frees it once nothing references it, so
// callers never free it and no call can overwrite the result of an earlier
// one. (Previously these functions returned shared static buffers; bug C1/C1b.)

namespace {

const char* gc_copy(const char* data, size_t len) {
  char* out = static_cast<char*>(GC_MALLOC_ATOMIC(len + 1));
  if (!out) {
    std::fputs("tspp runtime: out of memory\n", stderr);
    std::abort();
  }
  if (len > 0) std::memcpy(out, data, len);
  out[len] = '\0';
  return out;
}

const char* gc_string(const std::string& s) {
  return gc_copy(s.data(), s.size());
}

}  // namespace

extern "C" {

// ============================================================================
// Runtime lifecycle
// ============================================================================

// Called once by the generated `main` before any user code (and, from
// Phase 5 on, before the module's global initializers). Idempotent.
void tspp_runtime_init(void) {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;
  GC_INIT();
}

const char* tspp_sys_get_platform() {
  return gc_string(tspp::core::syscall::get_platform());
}

int tspp_sys_get_cpu_count() {
  return tspp::core::syscall::get_cpu_count();
}

const char* tspp_sys_get_cwd() {
  return gc_string(tspp::core::syscall::get_cwd());
}

int tspp_sys_set_cwd(const char* path) {
  return tspp::core::syscall::set_cwd(path);
}

const char* tspp_sys_system_call(const char* command) {
  if (!command) return gc_copy("", 0);
  return gc_string(tspp::core::syscall::system_call(command));
}

int tspp_sys_get_pid() {
  return tspp::core::syscall::get_pid();
}

void tspp_sys_exit_process(int code) {
  tspp::core::syscall::exit_process(code);
}

int tspp_sys_file_exists(const char* path) {
  return tspp::core::syscall::file_exists(path) ? 1 : 0;
}

const char* tspp_sys_read_file(const char* path) {
  if (!path) return gc_copy("", 0);
  return gc_string(tspp::core::syscall::read_file(path));
}

const char* tspp_sys_get_env(const char* name) {
  return gc_string(tspp::core::syscall::get_env(name ? name : ""));
}

int tspp_sys_set_env(const char* name, const char* value) {
  if (!name) return -1;
  return tspp::core::syscall::set_env(name, value ? value : "");
}

int tspp_sys_write_file(const char* path, const char* content, int append) {
  return tspp::core::syscall::write_file(path, content, append != 0);
}

int tspp_sys_delete_file(const char* path) {
  return tspp::core::syscall::delete_file(path);
}

int tspp_sys_is_directory(const char* path) {
  return tspp::core::syscall::is_directory(path) ? 1 : 0;
}

// ============================================================================
// Network C bindings
// ============================================================================

const char* tspp_sys_http_get(const char* url) {
  if (!url) return gc_copy("", 0);
  return gc_string(tspp::core::syscall::http_get(url));
}

int tspp_sys_tcp_connect(const char* host, int port) {
  if (!host) return -1;
  return tspp::core::syscall::tcp_connect(host, port);
}

int tspp_sys_tcp_send(int sockfd, const char* buf, int len) {
  if (!buf || len <= 0) return -1;
  return tspp::core::syscall::tcp_send(sockfd, std::string(buf, (size_t)len));
}

int tspp_sys_tcp_close(int sockfd) {
  return tspp::core::syscall::tcp_close(sockfd);
}

int tspp_sys_tcp_server_start(int port) {
  return tspp::core::syscall::tcp_server_start(port);
}
int tspp_sys_tcp_server_stop(int handle) {
  return tspp::core::syscall::tcp_server_stop(handle);
}
int tspp_sys_tcp_server_accept(int handle, int timeout_ms) {
  return tspp::core::syscall::tcp_server_accept(handle, timeout_ms);
}
const char* tspp_sys_tcp_recv(int sockfd, int maxlen) {
  return gc_string(tspp::core::syscall::tcp_recv(sockfd, maxlen));
}
int tspp_sys_tcp_recv_into(int sockfd, char* buf, int maxlen) {
  return tspp::core::syscall::tcp_recv_into(sockfd, buf, maxlen);
}
void* tspp_sys_malloc(int size) {
  return std::malloc((size_t)size);
}
void tspp_sys_free(void* p) {
  std::free(p);
}
const char* tspp_sys_string_from_buffer(const char* buf, int len) {
  if (!buf || len <= 0) return gc_copy("", 0);
  return gc_copy(buf, (size_t)len);
}

// ============================================================================
// Graphics C bindings
// ============================================================================

int tspp_sys_create_canvas(int width, int height) {
  return tspp::core::syscall::create_canvas(width, height);
}

int tspp_sys_draw_rect(int canvasHandle, int x, int y, int w, int h, int r,
                       int g, int b) {
  return tspp::core::syscall::draw_rect(canvasHandle, x, y, w, h, r, g, b);
}

const char* tspp_sys_present_canvas(int canvasHandle, const char* path) {
  std::string p = path ? path : std::string();
  return gc_string(tspp::core::syscall::present_canvas(canvasHandle, p));
}

// ============================================================================
// String Conversion & Console Functions
// ============================================================================

// 1 if both strings have the same contents, else 0. A null pointer equals only
// another null pointer, so `s == null` works.
int tspp_string_equals(const char* a, const char* b) {
  if (!a || !b) return a == b ? 1 : 0;
  return std::strcmp(a, b) == 0 ? 1 : 0;
}

const char* tspp_string_concat(const char* a, const char* b) {
  if (!a) a = "";
  if (!b) b = "";
  size_t la = std::strlen(a);
  size_t lb = std::strlen(b);
  char* out = static_cast<char*>(GC_MALLOC_ATOMIC(la + lb + 1));
  if (!out) {
    std::fputs("tspp runtime: out of memory\n", stderr);
    std::abort();
  }
  std::memcpy(out, a, la);
  std::memcpy(out + la, b, lb);
  out[la + lb] = '\0';
  return out;
}

// Compatibility shim: some codegen paths emit calls to
// `tspp_sys_int_to_string`; provide a trivial passthrough so linkage succeeds.
// It simply returns the integer unchanged so callers can forward it to
// `tspp_int_to_string`.
int tspp_sys_int_to_string(int v) {
  return v;
}

const char* tspp_int_to_string(int val) {
  return gc_string(std::to_string(val));
}

const char* tspp_bool_to_string(int val) {
  return val ? gc_copy("true", 4) : gc_copy("false", 5);
}

// Codegen passes tspp floats as 32-bit `float` (not `double`), so this takes a
// float. Prints the shortest decimal that reads back as the same float:
// 10, 1.1, 0.5, 100000 (never 1.100000).
const char* tspp_float_to_string(float val) {
  char buf[64];
  if (!std::isfinite(val)) {
    std::snprintf(buf, sizeof buf, "%g", static_cast<double>(val));
    return gc_string(buf);
  }
  for (int precision = 0; precision <= 9; ++precision) {
    std::snprintf(buf, sizeof buf, "%.*f", precision,
                  static_cast<double>(val));
    if (std::strtof(buf, nullptr) == val) return gc_string(buf);
  }
  // Very large/small magnitudes that fixed notation cannot round-trip.
  std::snprintf(buf, sizeof buf, "%.9g", static_cast<double>(val));
  return gc_string(buf);
}

const char* tspp_ptr_to_string(void* ptr) {
  std::stringstream ss;
  ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
  return gc_string(ss.str());
}

void tspp_console_log(const char* msg) {
  std::cout << (msg ? msg : "") << std::endl;
}

void tspp_console_error(const char* msg) {
  std::cerr << "\033[1;31m[error]\033[0m " << (msg ? msg : "") << std::endl;
}

void tspp_console_warn(const char* msg) {
  std::cerr << "\033[1;33m[warn]\033[0m " << (msg ? msg : "") << std::endl;
}

void tspp_free_string(void* p) {
  // No-op: runtime strings are GC-owned (see gc_copy) and string literals are
  // constants, so nothing here may be freed explicitly. Codegen still emits
  // calls to this after console.log; kept for ABI compatibility.
  (void)p;
}

}  // extern "C"
