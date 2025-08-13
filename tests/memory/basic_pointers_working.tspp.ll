; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 10
@pa = global ptr null
@b = global float 0x40091EB860000000
@pb = global ptr null
@c = global i1 true
@pc = global ptr null
@0 = private unnamed_addr constant [33 x i8] c"Testing basic pointer operations\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@2 = private unnamed_addr constant [16 x i8] c"Original int a:\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@4 = private unnamed_addr constant [22 x i8] c"Dereferenced int* pa:\00", align 1
@5 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@6 = private unnamed_addr constant [18 x i8] c"Original float b:\00", align 1
@7 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@8 = private unnamed_addr constant [24 x i8] c"Dereferenced float* pb:\00", align 1
@9 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@10 = private unnamed_addr constant [17 x i8] c"Original bool c:\00", align 1
@11 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@12 = private unnamed_addr constant [23 x i8] c"Dereferenced bool* pc:\00", align 1
@13 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  store ptr @a, ptr @pa, align 8
  store ptr @b, ptr @pb, align 8
  store ptr @c, ptr @pc, align 8
  ret void
}

define i1 @main() {
entry:
  %0 = call ptr @tspp_string_concat(ptr @1, ptr @0)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @1)
  call void @tspp_console_log(ptr %1)
  %2 = call ptr @tspp_string_concat(ptr @3, ptr @2)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @3)
  call void @tspp_console_log(ptr %3)
  %4 = load i32, ptr @a, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = call ptr @tspp_string_concat(ptr @5, ptr @4)
  %7 = call ptr @tspp_string_concat(ptr %6, ptr @5)
  call void @tspp_console_log(ptr %7)
  %8 = load ptr, ptr @pa, align 8
  %deref = load i32, ptr %8, align 4
  %9 = call ptr @tspp_int_to_string(i32 %deref)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %9)
  %10 = call ptr @tspp_string_concat(ptr @7, ptr @6)
  %11 = call ptr @tspp_string_concat(ptr %10, ptr @7)
  call void @tspp_console_log(ptr %11)
  %12 = load float, ptr @b, align 4
  %13 = call ptr @tspp_float_to_string(float %12)
  call void @tspp_console_log(ptr %13)
  call void @tspp_free_string(ptr %13)
  %14 = call ptr @tspp_string_concat(ptr @9, ptr @8)
  %15 = call ptr @tspp_string_concat(ptr %14, ptr @9)
  call void @tspp_console_log(ptr %15)
  %16 = load ptr, ptr @pb, align 8
  %deref1 = load float, ptr %16, align 4
  %17 = call ptr @tspp_float_to_string(float %deref1)
  call void @tspp_console_log(ptr %17)
  call void @tspp_free_string(ptr %17)
  %18 = call ptr @tspp_string_concat(ptr @11, ptr @10)
  %19 = call ptr @tspp_string_concat(ptr %18, ptr @11)
  call void @tspp_console_log(ptr %19)
  %20 = load i1, ptr @c, align 1
  %21 = call ptr @tspp_bool_to_string(i1 %20)
  call void @tspp_console_log(ptr %21)
  call void @tspp_free_string(ptr %21)
  %22 = call ptr @tspp_string_concat(ptr @13, ptr @12)
  %23 = call ptr @tspp_string_concat(ptr %22, ptr @13)
  call void @tspp_console_log(ptr %23)
  %24 = load ptr, ptr @pc, align 8
  %deref2 = load i1, ptr %24, align 1
  %25 = call ptr @tspp_bool_to_string(i1 %deref2)
  call void @tspp_console_log(ptr %25)
  call void @tspp_free_string(ptr %25)
  ret i1 true
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_bool_to_string(i1)
