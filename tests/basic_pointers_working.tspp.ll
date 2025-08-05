; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 10
@pa = global ptr @a
@b = global float 0x40091EB860000000
@pb = global ptr @b
@c = global i1 true
@pc = global ptr @c
@0 = private unnamed_addr constant [35 x i8] c"\22Testing basic pointer operations\22\00", align 1
@1 = private unnamed_addr constant [18 x i8] c"\22Original int a:\22\00", align 1
@2 = private unnamed_addr constant [24 x i8] c"\22Dereferenced int* pa:\22\00", align 1
@3 = private unnamed_addr constant [20 x i8] c"\22Original float b:\22\00", align 1
@4 = private unnamed_addr constant [26 x i8] c"\22Dereferenced float* pb:\22\00", align 1
@5 = private unnamed_addr constant [19 x i8] c"\22Original bool c:\22\00", align 1
@6 = private unnamed_addr constant [25 x i8] c"\22Dereferenced bool* pc:\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  call void @tspp_console_log(ptr @1)
  %0 = load i32, ptr @a, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  call void @tspp_console_log(ptr @2)
  %2 = load ptr, ptr @pa, align 8
  %deref = load i32, ptr %2, align 4
  %3 = call ptr @tspp_int_to_string(i32 %deref)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  call void @tspp_console_log(ptr @3)
  %4 = load float, ptr @b, align 4
  %5 = call ptr @tspp_float_to_string(float %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  call void @tspp_console_log(ptr @4)
  %6 = load ptr, ptr @pb, align 8
  %deref1 = load float, ptr %6, align 4
  %7 = call ptr @tspp_float_to_string(float %deref1)
  call void @tspp_console_log(ptr %7)
  call void @tspp_free_string(ptr %7)
  call void @tspp_console_log(ptr @5)
  %8 = load i1, ptr @c, align 1
  %9 = call ptr @tspp_bool_to_string(i1 %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %9)
  call void @tspp_console_log(ptr @6)
  %10 = load ptr, ptr @pc, align 8
  %deref2 = load i1, ptr %10, align 1
  %11 = call ptr @tspp_bool_to_string(i1 %deref2)
  call void @tspp_console_log(ptr %11)
  call void @tspp_free_string(ptr %11)
  ret i1 true
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_bool_to_string(i1)
