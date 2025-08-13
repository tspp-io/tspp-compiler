; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 42
@pa = global i32 0
@val = global i32 0
@b = global float 0x40091EB860000000
@pb = global i32 0
@fval = global float 0.000000e+00
@0 = private unnamed_addr constant [33 x i8] c"Testing basic pointer operations\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  %0 = load i32, ptr @pa, align 4
  %1 = load i32, ptr @pb, align 4
  ret void
}

define i1 @main() {
entry:
  %0 = call ptr @tspp_string_concat(ptr @1, ptr @0)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @1)
  call void @tspp_console_log(ptr %1)
  %2 = load i32, ptr @val, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = load float, ptr @fval, align 4
  %5 = call ptr @tspp_float_to_string(float %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  ret i1 false
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_float_to_string(float)
