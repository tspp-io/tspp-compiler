#include "io.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

// Error constants
const char* TSPP_IO_EOF = "EOF";
const char* TSPP_IO_UNEXPECTED_EOF = "unexpected EOF";

bool tspp_io_is_eof(int err) {
    return err == 0; // In our convention, 0 bytes read indicates EOF
}

// Basic I/O utility functions
int tspp_io_copy(TsppWriter* dst, TsppReader* src) {
    char buffer[4096];
    int total_written = 0;
    
    while (true) {
        int bytes_read = src->read(src->data, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // EOF or error
        }
        
        int bytes_written = dst->write(dst->data, buffer, bytes_read);
        if (bytes_written < 0) {
            return bytes_written; // Error
        }
        
        total_written += bytes_written;
        
        if (bytes_written != bytes_read) {
            break; // Partial write
        }
    }
    
    return total_written;
}

int tspp_io_copy_n(TsppWriter* dst, TsppReader* src, int64_t n) {
    char buffer[4096];
    int64_t remaining = n;
    int total_written = 0;
    
    while (remaining > 0) {
        size_t to_read = std::min((int64_t)sizeof(buffer), remaining);
        int bytes_read = src->read(src->data, buffer, to_read);
        if (bytes_read <= 0) {
            break; // EOF or error
        }
        
        int bytes_written = dst->write(dst->data, buffer, bytes_read);
        if (bytes_written < 0) {
            return bytes_written; // Error
        }
        
        total_written += bytes_written;
        remaining -= bytes_written;
        
        if (bytes_written != bytes_read) {
            break; // Partial write
        }
    }
    
    return total_written;
}

int tspp_io_read_all(TsppReader* r, char** buf, size_t* size) {
    if (!r || !buf || !size) return -1;
    
    std::vector<char> data;
    char buffer[4096];
    
    while (true) {
        int bytes_read = r->read(r->data, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // EOF or error
        }
        
        data.insert(data.end(), buffer, buffer + bytes_read);
    }
    
    *size = data.size();
    if (*size == 0) {
        *buf = nullptr;
        return 0;
    }
    
    *buf = (char*)malloc(*size);
    if (!*buf) return -1;
    
    memcpy(*buf, data.data(), *size);
    return *size;
}

int tspp_io_read_full(TsppReader* r, char* buf, size_t count) {
    if (!r || !buf) return -1;
    
    size_t total_read = 0;
    while (total_read < count) {
        int bytes_read = r->read(r->data, buf + total_read, count - total_read);
        if (bytes_read <= 0) {
            break; // EOF or error
        }
        total_read += bytes_read;
    }
    
    return total_read;
}

int tspp_io_write_string(TsppWriter* w, const char* s) {
    if (!w || !s) return -1;
    
    size_t len = strlen(s);
    return w->write(w->data, s, len);
}

// Multi-reader implementation
static int multi_reader_read(void* data, char* buf, size_t count) {
    TsppMultiReader* mr = (TsppMultiReader*)data;
    
    while (mr->current < mr->count) {
        int bytes_read = mr->readers[mr->current]->read(
            mr->readers[mr->current]->data, buf, count);
        
        if (bytes_read > 0) {
            return bytes_read;
        }
        
        // Move to next reader
        mr->current++;
    }
    
    return 0; // EOF
}

static void multi_reader_close(void* data) {
    // Multi-reader doesn't own the individual readers
}

TsppMultiReader* tspp_io_multi_reader(TsppReader** readers, size_t count) {
    if (!readers || count == 0) return nullptr;
    
    TsppMultiReader* mr = (TsppMultiReader*)malloc(sizeof(TsppMultiReader));
    if (!mr) return nullptr;
    
    mr->readers = readers;
    mr->count = count;
    mr->current = 0;
    
    return mr;
}

void tspp_io_multi_reader_free(TsppMultiReader* mr) {
    free(mr);
}

// Multi-writer implementation  
static int multi_writer_write(void* data, const char* buf, size_t count) {
    TsppMultiWriter* mw = (TsppMultiWriter*)data;
    
    int min_written = count;
    for (size_t i = 0; i < mw->count; i++) {
        int bytes_written = mw->writers[i]->write(mw->writers[i]->data, buf, count);
        if (bytes_written < 0) {
            return bytes_written; // Error
        }
        min_written = std::min(min_written, bytes_written);
    }
    
    return min_written;
}

static void multi_writer_close(void* data) {
    // Multi-writer doesn't own the individual writers
}

TsppMultiWriter* tspp_io_multi_writer(TsppWriter** writers, size_t count) {
    if (!writers || count == 0) return nullptr;
    
    TsppMultiWriter* mw = (TsppMultiWriter*)malloc(sizeof(TsppMultiWriter));
    if (!mw) return nullptr;
    
    mw->writers = writers;
    mw->count = count;
    
    return mw;
}

void tspp_io_multi_writer_free(TsppMultiWriter* mw) {
    free(mw);
}

// Standard I/O implementations
static int stdin_read(void* data, char* buf, size_t count) {
    size_t bytes_read = fread(buf, 1, count, stdin);
    return bytes_read > 0 ? bytes_read : (feof(stdin) ? 0 : -1);
}

static void stdin_close(void* data) {
    // Don't actually close stdin
}

static int stdout_write(void* data, const char* buf, size_t count) {
    size_t bytes_written = fwrite(buf, 1, count, stdout);
    fflush(stdout);
    return bytes_written;
}

static void stdout_close(void* data) {
    // Don't actually close stdout
}

static int stderr_write(void* data, const char* buf, size_t count) {
    size_t bytes_written = fwrite(buf, 1, count, stderr);
    fflush(stderr);
    return bytes_written;
}

static void stderr_close(void* data) {
    // Don't actually close stderr  
}

TsppReader* tspp_io_stdin_reader() {
    static TsppReader stdin_reader = { nullptr, stdin_read, stdin_close };
    return &stdin_reader;
}

TsppWriter* tspp_io_stdout_writer() {
    static TsppWriter stdout_writer = { nullptr, stdout_write, stdout_close };
    return &stdout_writer;
}

TsppWriter* tspp_io_stderr_writer() {
    static TsppWriter stderr_writer = { nullptr, stderr_write, stderr_close };
    return &stderr_writer;
}

// String reader implementation
static int string_reader_read(void* data, char* buf, size_t count) {
    TsppStringReader* sr = (TsppStringReader*)data;
    
    if (sr->pos >= sr->len) {
        return 0; // EOF
    }
    
    size_t to_read = std::min(count, sr->len - sr->pos);
    memcpy(buf, sr->data + sr->pos, to_read);
    sr->pos += to_read;
    
    return to_read;
}

static void string_reader_close(void* data) {
    // String reader doesn't need cleanup
}

TsppStringReader* tspp_io_new_string_reader(const char* s) {
    if (!s) return nullptr;
    
    TsppStringReader* sr = (TsppStringReader*)malloc(sizeof(TsppStringReader));
    if (!sr) return nullptr;
    
    sr->data = s;
    sr->pos = 0;
    sr->len = strlen(s);
    
    return sr;
}

int tspp_io_string_reader_read(TsppStringReader* sr, char* buf, size_t count) {
    if (!sr) return -1;
    return string_reader_read(sr, buf, count);
}

void tspp_io_string_reader_free(TsppStringReader* sr) {
    free(sr);
}

// String writer implementation
static int string_writer_write(void* data, const char* buf, size_t count) {
    TsppStringWriter* sw = (TsppStringWriter*)data;
    
    // Ensure capacity
    if (sw->len + count > sw->cap) {
        size_t new_cap = std::max(sw->cap * 2, sw->len + count);
        char* new_data = (char*)realloc(sw->data, new_cap);
        if (!new_data) return -1;
        
        sw->data = new_data;
        sw->cap = new_cap;
    }
    
    memcpy(sw->data + sw->len, buf, count);
    sw->len += count;
    
    return count;
}

static void string_writer_close(void* data) {
    // String writer doesn't need special close handling
}

TsppStringWriter* tspp_io_new_string_writer() {
    return tspp_io_new_string_writer_cap(256);
}

TsppStringWriter* tspp_io_new_string_writer_cap(size_t cap) {
    TsppStringWriter* sw = (TsppStringWriter*)malloc(sizeof(TsppStringWriter));
    if (!sw) return nullptr;
    
    sw->data = (char*)malloc(cap);
    if (!sw->data) {
        free(sw);
        return nullptr;
    }
    
    sw->len = 0;
    sw->cap = cap;
    
    return sw;
}

int tspp_io_string_writer_write(TsppStringWriter* sw, const char* buf, size_t count) {
    if (!sw) return -1;
    return string_writer_write(sw, buf, count);
}

char* tspp_io_string_writer_string(TsppStringWriter* sw) {
    if (!sw) return nullptr;
    
    char* result = (char*)malloc(sw->len + 1);
    if (!result) return nullptr;
    
    memcpy(result, sw->data, sw->len);
    result[sw->len] = '\0';
    
    return result;
}

void tspp_io_string_writer_free(TsppStringWriter* sw) {
    if (sw) {
        free(sw->data);
        free(sw);
    }
}

// Buffered reader implementation
TsppBufReader* tspp_io_new_buf_reader(TsppReader* r) {
    return tspp_io_new_buf_reader_size(r, 4096);
}

TsppBufReader* tspp_io_new_buf_reader_size(TsppReader* r, size_t size) {
    if (!r) return nullptr;
    
    TsppBufReader* br = (TsppBufReader*)malloc(sizeof(TsppBufReader));
    if (!br) return nullptr;
    
    br->buf = (char*)malloc(size);
    if (!br->buf) {
        free(br);
        return nullptr;
    }
    
    br->reader = r;
    br->buf_size = size;
    br->buf_pos = 0;
    br->buf_len = 0;
    
    return br;
}

int tspp_io_buf_reader_read(TsppBufReader* br, char* buf, size_t count) {
    if (!br || !buf) return -1;
    
    // If buffer is empty, fill it
    if (br->buf_pos >= br->buf_len) {
        br->buf_pos = 0;
        int bytes_read = br->reader->read(br->reader->data, br->buf, br->buf_size);
        if (bytes_read <= 0) {
            br->buf_len = 0;
            return bytes_read;
        }
        br->buf_len = bytes_read;
    }
    
    // Copy from buffer
    size_t available = br->buf_len - br->buf_pos;
    size_t to_copy = std::min(count, available);
    
    memcpy(buf, br->buf + br->buf_pos, to_copy);
    br->buf_pos += to_copy;
    
    return to_copy;
}

int tspp_io_buf_reader_read_byte(TsppBufReader* br) {
    char c;
    int result = tspp_io_buf_reader_read(br, &c, 1);
    return result == 1 ? (unsigned char)c : -1;
}

void tspp_io_buf_reader_free(TsppBufReader* br) {
    if (br) {
        free(br->buf);
        free(br);
    }
}

// Buffered writer implementation
TsppBufWriter* tspp_io_new_buf_writer(TsppWriter* w) {
    return tspp_io_new_buf_writer_size(w, 4096);
}

TsppBufWriter* tspp_io_new_buf_writer_size(TsppWriter* w, size_t size) {
    if (!w) return nullptr;
    
    TsppBufWriter* bw = (TsppBufWriter*)malloc(sizeof(TsppBufWriter));
    if (!bw) return nullptr;
    
    bw->buf = (char*)malloc(size);
    if (!bw->buf) {
        free(bw);
        return nullptr;
    }
    
    bw->writer = w;
    bw->buf_size = size;
    bw->buf_len = 0;
    
    return bw;
}

int tspp_io_buf_writer_write(TsppBufWriter* bw, const char* buf, size_t count) {
    if (!bw || !buf) return -1;
    
    // If write is larger than buffer, flush and write directly
    if (count > bw->buf_size) {
        if (tspp_io_buf_writer_flush(bw) < 0) {
            return -1;
        }
        return bw->writer->write(bw->writer->data, buf, count);
    }
    
    // If buffer would overflow, flush first
    if (bw->buf_len + count > bw->buf_size) {
        if (tspp_io_buf_writer_flush(bw) < 0) {
            return -1;
        }
    }
    
    // Add to buffer
    memcpy(bw->buf + bw->buf_len, buf, count);
    bw->buf_len += count;
    
    return count;
}

int tspp_io_buf_writer_write_byte(TsppBufWriter* bw, char c) {
    return tspp_io_buf_writer_write(bw, &c, 1);
}

int tspp_io_buf_writer_write_string(TsppBufWriter* bw, const char* s) {
    if (!s) return 0;
    return tspp_io_buf_writer_write(bw, s, strlen(s));
}

int tspp_io_buf_writer_flush(TsppBufWriter* bw) {
    if (!bw || bw->buf_len == 0) return 0;
    
    int bytes_written = bw->writer->write(bw->writer->data, bw->buf, bw->buf_len);
    if (bytes_written > 0) {
        bw->buf_len = 0;
    }
    
    return bytes_written;
}

void tspp_io_buf_writer_free(TsppBufWriter* bw) {
    if (bw) {
        tspp_io_buf_writer_flush(bw); // Flush before freeing
        free(bw->buf);
        free(bw);
    }
}