; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@x = global i32 10
@y = global i32 20
@z = global i32 5
@sum = global i32 30
@product = global i32 200
@difference = global i32 10
@quotient = global i32 4
@remainder = global i32 0
@complex1 = global i32 150
@complex2 = global i32 205
@complex3 = global i32 125
@flag1 = global i1 false
@flag2 = global i1 true
@flag3 = global i1 true
@flag4 = global i1 true
@flag5 = global i1 true
@flag6 = global i1 true
@0 = private unnamed_addr constant [12 x i8] c"Arithmetic:\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@2 = private unnamed_addr constant [9 x i8] c"Complex:\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@4 = private unnamed_addr constant [9 x i8] c"Boolean:\00", align 1
@5 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@6 = private unnamed_addr constant [9 x i8] c"Logical:\00", align 1
@7 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i1 @main() {
entry:
  %0 = call ptr @tspp_string_concat(ptr @1, ptr @0)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @1)
  call void @tspp_console_log(ptr %1)
  %2 = load i32, ptr @sum, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = load i32, ptr @product, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = load i32, ptr @difference, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  call void @tspp_console_log(ptr %7)
  call void @tspp_free_string(ptr %7)
  %8 = load i32, ptr @quotient, align 4
  %9 = call ptr @tspp_int_to_string(i32 %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %9)
  %10 = load i32, ptr @remainder, align 4
  %11 = call ptr @tspp_int_to_string(i32 %10)
  call void @tspp_console_log(ptr %11)
  call void @tspp_free_string(ptr %11)
  %12 = call ptr @tspp_string_concat(ptr @3, ptr @2)
  %13 = call ptr @tspp_string_concat(ptr %12, ptr @3)
  call void @tspp_console_log(ptr %13)
  %14 = load i32, ptr @complex1, align 4
  %15 = call ptr @tspp_int_to_string(i32 %14)
  call void @tspp_console_log(ptr %15)
  call void @tspp_free_string(ptr %15)
  %16 = load i32, ptr @complex2, align 4
  %17 = call ptr @tspp_int_to_string(i32 %16)
  call void @tspp_console_log(ptr %17)
  call void @tspp_free_string(ptr %17)
  %18 = load i32, ptr @complex3, align 4
  %19 = call ptr @tspp_int_to_string(i32 %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %19)
  %20 = call ptr @tspp_string_concat(ptr @5, ptr @4)
  %21 = call ptr @tspp_string_concat(ptr %20, ptr @5)
  call void @tspp_console_log(ptr %21)
  %22 = load i1, ptr @flag1, align 1
  %23 = call ptr @tspp_bool_to_string(i1 %22)
  call void @tspp_console_log(ptr %23)
  call void @tspp_free_string(ptr %23)
  %24 = load i1, ptr @flag2, align 1
  %25 = call ptr @tspp_bool_to_string(i1 %24)
  call void @tspp_console_log(ptr %25)
  call void @tspp_free_string(ptr %25)
  %26 = load i1, ptr @flag3, align 1
  %27 = call ptr @tspp_bool_to_string(i1 %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %27)
  %28 = load i1, ptr @flag4, align 1
  %29 = call ptr @tspp_bool_to_string(i1 %28)
  call void @tspp_console_log(ptr %29)
  call void @tspp_free_string(ptr %29)
  %30 = call ptr @tspp_string_concat(ptr @7, ptr @6)
  %31 = call ptr @tspp_string_concat(ptr %30, ptr @7)
  call void @tspp_console_log(ptr %31)
  %32 = load i1, ptr @flag5, align 1
  %33 = call ptr @tspp_bool_to_string(i1 %32)
  call void @tspp_console_log(ptr %33)
  call void @tspp_free_string(ptr %33)
  %34 = load i1, ptr @flag6, align 1
  %35 = call ptr @tspp_bool_to_string(i1 %34)
  call void @tspp_console_log(ptr %35)
  call void @tspp_free_string(ptr %35)
  ret i1 false
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
