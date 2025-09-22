#include "syscall.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>

extern "C" {

// Generic syscall implementations
long tspp_syscall0(long number) {
    return syscall(number);
}

long tspp_syscall1(long number, long arg1) {
    return syscall(number, arg1);
}

long tspp_syscall2(long number, long arg1, long arg2) {
    return syscall(number, arg1, arg2);
}

long tspp_syscall3(long number, long arg1, long arg2, long arg3) {
    return syscall(number, arg1, arg2, arg3);
}

long tspp_syscall4(long number, long arg1, long arg2, long arg3, long arg4) {
    return syscall(number, arg1, arg2, arg3, arg4);
}

long tspp_syscall5(long number, long arg1, long arg2, long arg3, long arg4, long arg5) {
    return syscall(number, arg1, arg2, arg3, arg4, arg5);
}

long tspp_syscall6(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    return syscall(number, arg1, arg2, arg3, arg4, arg5, arg6);
}

// Common syscall wrappers
int tspp_sys_open(const char* pathname, int flags, int mode) {
    return static_cast<int>(syscall(SYS_open, pathname, flags, mode));
}

long tspp_sys_read(int fd, char* buf, unsigned long count) {
    return syscall(SYS_read, fd, buf, count);
}

long tspp_sys_write(int fd, const char* buf, unsigned long count) {
    return syscall(SYS_write, fd, buf, count);
}

int tspp_sys_close(int fd) {
    return static_cast<int>(syscall(SYS_close, fd));
}

int tspp_sys_exit(int status) {
    syscall(SYS_exit, status);
    return 0; // Never reached, but keeps compiler happy
}

}  // extern "C"