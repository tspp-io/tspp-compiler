#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdint>

// Basic I/O interfaces
struct TsppReader {
    void* data;
    int (*read)(void* data, char* buf, size_t count);
    void (*close)(void* data);
};

struct TsppWriter {
    void* data;
    int (*write)(void* data, const char* buf, size_t count);
    void (*close)(void* data);
};

struct TsppCloser {
    void* data;
    void (*close)(void* data);
};

struct TsppSeeker {
    void* data;
    int64_t (*seek)(void* data, int64_t offset, int whence);
};

struct TsppReadWriter {
    TsppReader reader;
    TsppWriter writer;
};

struct TsppReadCloser {
    TsppReader reader;
    TsppCloser closer;
};

struct TsppWriteCloser {
    TsppWriter writer;
    TsppCloser closer;
};

struct TsppReadWriteCloser {
    TsppReader reader;
    TsppWriter writer;
    TsppCloser closer;
};

struct TsppReadSeeker {
    TsppReader reader;
    TsppSeeker seeker;
};

struct TsppWriteSeeker {
    TsppWriter writer;
    TsppSeeker seeker;
};

struct TsppReadWriteSeeker {
    TsppReader reader;
    TsppWriter writer;
    TsppSeeker seeker;
};

// Multi-reader/writer
struct TsppMultiReader {
    TsppReader** readers;
    size_t count;
    size_t current;
};

struct TsppMultiWriter {
    TsppWriter** writers;
    size_t count;
};

// Pipe implementation
struct TsppPipe {
    TsppReader* reader;
    TsppWriter* writer;
};

// Buffered I/O
struct TsppBufReader {
    TsppReader* reader;
    char* buf;
    size_t buf_size;
    size_t buf_pos;
    size_t buf_len;
};

struct TsppBufWriter {
    TsppWriter* writer;
    char* buf;
    size_t buf_size;
    size_t buf_len;
};

// String readers and writers
struct TsppStringReader {
    const char* data;
    size_t pos;
    size_t len;
};

struct TsppStringWriter {
    char* data;
    size_t len;
    size_t cap;
};

extern "C" {
    // I/O utility functions
    int tspp_io_copy(TsppWriter* dst, TsppReader* src);
    int tspp_io_copy_n(TsppWriter* dst, TsppReader* src, int64_t n);
    int tspp_io_copy_buffer(TsppWriter* dst, TsppReader* src, char* buf, size_t buf_size);

    // Read functions
    int tspp_io_read_all(TsppReader* r, char** buf, size_t* size);
    int tspp_io_read_full(TsppReader* r, char* buf, size_t count);

    // Write functions
    int tspp_io_write_string(TsppWriter* w, const char* s);

    // Multi-reader/writer functions
    TsppMultiReader* tspp_io_multi_reader(TsppReader** readers, size_t count);
    TsppMultiWriter* tspp_io_multi_writer(TsppWriter** writers, size_t count);
    void tspp_io_multi_reader_free(TsppMultiReader* mr);
    void tspp_io_multi_writer_free(TsppMultiWriter* mw);

    // Pipe functions
    TsppPipe* tspp_io_pipe();
    void tspp_io_pipe_free(TsppPipe* p);

    // Buffered I/O functions
    TsppBufReader* tspp_io_new_buf_reader(TsppReader* r);
    TsppBufReader* tspp_io_new_buf_reader_size(TsppReader* r, size_t size);
    TsppBufWriter* tspp_io_new_buf_writer(TsppWriter* w);
    TsppBufWriter* tspp_io_new_buf_writer_size(TsppWriter* w, size_t size);

    int tspp_io_buf_reader_read(TsppBufReader* br, char* buf, size_t count);
    int tspp_io_buf_reader_read_byte(TsppBufReader* br);
    int tspp_io_buf_reader_read_line(TsppBufReader* br, char** line, size_t* size);
    int tspp_io_buf_reader_peek(TsppBufReader* br, char* buf, size_t count);

    int tspp_io_buf_writer_write(TsppBufWriter* bw, const char* buf, size_t count);
    int tspp_io_buf_writer_write_byte(TsppBufWriter* bw, char c);
    int tspp_io_buf_writer_write_string(TsppBufWriter* bw, const char* s);
    int tspp_io_buf_writer_flush(TsppBufWriter* bw);

    void tspp_io_buf_reader_free(TsppBufReader* br);
    void tspp_io_buf_writer_free(TsppBufWriter* bw);

    // Standard I/O wrappers
    TsppReader* tspp_io_stdin_reader();
    TsppWriter* tspp_io_stdout_writer();
    TsppWriter* tspp_io_stderr_writer();

    // File I/O wrappers  
    TsppReadWriteCloser* tspp_io_file_open(const char* filename, const char* mode);
    void tspp_io_file_close(TsppReadWriteCloser* file);

    // String I/O functions
    TsppStringReader* tspp_io_new_string_reader(const char* s);
    TsppStringWriter* tspp_io_new_string_writer();
    TsppStringWriter* tspp_io_new_string_writer_cap(size_t cap);

    int tspp_io_string_reader_read(TsppStringReader* sr, char* buf, size_t count);
    int tspp_io_string_writer_write(TsppStringWriter* sw, const char* buf, size_t count);
    char* tspp_io_string_writer_string(TsppStringWriter* sw);

    void tspp_io_string_reader_free(TsppStringReader* sr);
    void tspp_io_string_writer_free(TsppStringWriter* sw);

    // Error constants and functions
    extern const char* TSPP_IO_EOF;
    extern const char* TSPP_IO_UNEXPECTED_EOF;
    bool tspp_io_is_eof(int err);
}