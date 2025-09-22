#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// Generic syscall interface
long tspp_syscall0(long number);
long tspp_syscall1(long number, long arg1);
long tspp_syscall2(long number, long arg1, long arg2);
long tspp_syscall3(long number, long arg1, long arg2, long arg3);
long tspp_syscall4(long number, long arg1, long arg2, long arg3, long arg4);
long tspp_syscall5(long number, long arg1, long arg2, long arg3, long arg4, long arg5);
long tspp_syscall6(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

// Common syscall wrappers
int tspp_sys_open(const char* pathname, int flags, int mode);
long tspp_sys_read(int fd, char* buf, unsigned long count);
long tspp_sys_write(int fd, const char* buf, unsigned long count);
int tspp_sys_close(int fd);
int tspp_sys_exit(int status);

#ifdef __cplusplus
}
#endif