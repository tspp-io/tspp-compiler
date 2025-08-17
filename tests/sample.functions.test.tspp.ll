; ModuleID = 'tspp_module'
source_filename = "tspp_module"
target triple = "x86_64-pc-linux-gnu"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [8 x i8] c"Hello, \00", align 1
@1 = private unnamed_addr constant [2 x i8] c"(\00", align 1
@2 = private unnamed_addr constant [3 x i8] c", \00", align 1
@3 = private unnamed_addr constant [2 x i8] c")\00", align 1
@4 = private unnamed_addr constant [13 x i8] c"logMessage: \00", align 1
@5 = private unnamed_addr constant [11 x i8] c"add(2,3): \00", align 1
@6 = private unnamed_addr constant [10 x i8] c"greet(): \00", align 1
@7 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@8 = private unnamed_addr constant [17 x i8] c"greet('Alice'): \00", align 1
@9 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@10 = private unnamed_addr constant [21 x i8] c"multiply(2.5, 4.0): \00", align 1
@11 = private unnamed_addr constant [19 x i8] c"formatPoint(1.1): \00", align 1
@12 = private unnamed_addr constant [2 x i8] c"P\00", align 1
@13 = private unnamed_addr constant [28 x i8] c"formatPoint(2.2, 3.3,'Q'): \00", align 1
@14 = private unnamed_addr constant [2 x i8] c"Q\00", align 1
@15 = private unnamed_addr constant [12 x i8] c"square(5): \00", align 1
@16 = private unnamed_addr constant [15 x i8] c"increment(7): \00", align 1
@17 = private unnamed_addr constant [19 x i8] c"This is a test log\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @add(i32 %x, i32 %y) {
entry:
  %y2 = alloca i32, align 4
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i32 %y, ptr %y2, align 4
  %0 = load i32, ptr %x1, align 4
  %1 = load i32, ptr %y2, align 4
  %addtmp = add i32 %0, %1
  ret i32 %addtmp
}

define ptr @greet(ptr %name) {
entry:
  %name1 = alloca ptr, align 8
  store ptr %name, ptr %name1, align 8
  %0 = load ptr, ptr %name1, align 8
  %1 = call ptr @tspp_string_concat(ptr @0, ptr %0)
  ret ptr %1
}

declare ptr @tspp_string_concat(ptr, ptr)

define float @multiply(float %a, float %b) {
entry:
  %b2 = alloca float, align 4
  %a1 = alloca float, align 4
  store float %a, ptr %a1, align 4
  store float %b, ptr %b2, align 4
  %0 = load float, ptr %a1, align 4
  %1 = load float, ptr %b2, align 4
  %fmultmp = fmul float %0, %1
  ret float %fmultmp
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
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @1)
  %2 = load float, ptr %x1, align 4
  %3 = call ptr @tspp_float_to_string(float %2)
  %4 = call ptr @tspp_string_concat(ptr %1, ptr %3)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @2)
  %6 = load float, ptr %y2, align 4
  %7 = call ptr @tspp_float_to_string(float %6)
  %8 = call ptr @tspp_string_concat(ptr %5, ptr %7)
  %9 = call ptr @tspp_string_concat(ptr %8, ptr @3)
  ret ptr %9
}

declare ptr @tspp_float_to_string(float)

define i32 @square(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  %1 = load i32, ptr %n1, align 4
  %multmp = mul i32 %0, %1
  ret i32 %multmp
}

define i32 @increment(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %0 = load i32, ptr %x1, align 4
  %addtmp = add i32 %0, 1
  ret i32 %addtmp
}

define void @logMessage(ptr %msg) {
entry:
  %msg1 = alloca ptr, align 8
  store ptr %msg, ptr %msg1, align 8
  %0 = load ptr, ptr %msg1, align 8
  %1 = call ptr @tspp_string_concat(ptr @4, ptr %0)
  call void @tspp_console_log(ptr %1)
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

define i32 @main() {
entry:
  %0 = call i32 @add(i32 2, i32 3)
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call ptr @tspp_string_concat(ptr @5, ptr %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %1)
  %3 = call ptr @greet(ptr @7)
  %4 = call ptr @tspp_string_concat(ptr @6, ptr %3)
  call void @tspp_console_log(ptr %4)
  %5 = call ptr @greet(ptr @9)
  %6 = call ptr @tspp_string_concat(ptr @8, ptr %5)
  call void @tspp_console_log(ptr %6)
  %7 = call float @multiply(float 2.500000e+00, float 4.000000e+00)
  %8 = call ptr @tspp_float_to_string(float %7)
  %9 = call ptr @tspp_string_concat(ptr @10, ptr %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %8)
  %10 = call ptr @formatPoint(float 0x3FF19999A0000000, float 0.000000e+00, ptr @12)
  %11 = call ptr @tspp_string_concat(ptr @11, ptr %10)
  call void @tspp_console_log(ptr %11)
  %12 = call ptr @formatPoint(float 0x40019999A0000000, float 0x400A666660000000, ptr @14)
  %13 = call ptr @tspp_string_concat(ptr @13, ptr %12)
  call void @tspp_console_log(ptr %13)
  %14 = call i32 @square(i32 5)
  %15 = call ptr @tspp_int_to_string(i32 %14)
  %16 = call ptr @tspp_string_concat(ptr @15, ptr %15)
  call void @tspp_console_log(ptr %16)
  call void @tspp_free_string(ptr %15)
  %17 = call i32 @increment(i32 7)
  %18 = call ptr @tspp_int_to_string(i32 %17)
  %19 = call ptr @tspp_string_concat(ptr @16, ptr %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %18)
  call void @logMessage(ptr @17)
  ret i32 0
}

declare ptr @tspp_int_to_string(i32)
