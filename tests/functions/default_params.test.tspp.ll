; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [2 x i8] c"(\00", align 1
@1 = private unnamed_addr constant [3 x i8] c", \00", align 1
@2 = private unnamed_addr constant [2 x i8] c")\00", align 1
@3 = private unnamed_addr constant [19 x i8] c"formatPoint(1.1): \00", align 1
@4 = private unnamed_addr constant [2 x i8] c"P\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define ptr @formatPoint(float %x, float %y, ptr %label) {
entry:
  %label3 = alloca ptr, align 8
  %y2 = alloca float, align 4
  %x1 = alloca float, align 4
  store float %x, ptr %x1, align 4
  store float %y, ptr %y2, align 4
  store ptr %label, ptr %label3, align 8
  %0 = load ptr, ptr %label3, align 8
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @0)
  %2 = load float, ptr %x1, align 4
  %3 = call ptr @tspp_float_to_string(float %2)
  %4 = call ptr @tspp_string_concat(ptr %1, ptr %3)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @1)
  %6 = load float, ptr %y2, align 4
  %7 = call ptr @tspp_float_to_string(float %6)
  %8 = call ptr @tspp_string_concat(ptr %5, ptr %7)
  %9 = call ptr @tspp_string_concat(ptr %8, ptr @2)
  ret ptr %9
}

declare ptr @tspp_string_concat(ptr, ptr)

declare ptr @tspp_float_to_string(float)

define void @main_tspp() {
entry:
  %0 = call ptr @formatPoint(float 0x3FF19999A0000000, float 0.000000e+00, ptr @4)
  %1 = call ptr @tspp_string_concat(ptr @3, ptr %0)
  call void @tspp_console_log(ptr %1)
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
