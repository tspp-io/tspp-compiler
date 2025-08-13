; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [13 x i8] c"Hello, world\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@2 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @add() {
entry:
  ret i32 3
}

define i1 @isEven() {
entry:
  ret i1 true
}

define ptr @greet() {
entry:
  ret ptr @0
}

define float @average() {
entry:
  ret float 4.000000e+00
}

define ptr @getPointer() {
entry:
  %value = alloca i32, align 4
  store i32 10, ptr %value, align 4
  ret ptr %value
}

define ptr @getUnion() {
entry:
  %0 = call ptr @tspp_int_to_string(i32 42)
  ret ptr %0
}

declare ptr @tspp_int_to_string(i32)

define i32 @getConstValue() #0 {
entry:
  ret i32 100
}

define void @main_tspp() {
entry:
  %0 = call i32 @add()
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = call i1 @isEven()
  %3 = call ptr @tspp_bool_to_string(i1 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = call ptr @greet()
  %5 = call ptr @tspp_string_concat(ptr @1, ptr %4)
  %6 = call ptr @tspp_string_concat(ptr %5, ptr @1)
  call void @tspp_console_log(ptr %6)
  %7 = call float @average()
  %8 = call ptr @tspp_float_to_string(float %7)
  call void @tspp_console_log(ptr %8)
  call void @tspp_free_string(ptr %8)
  %9 = call ptr @getPointer()
  %10 = call ptr @tspp_ptr_to_string(ptr %9)
  call void @tspp_console_log(ptr %10)
  call void @tspp_free_string(ptr %10)
  %11 = call ptr @getUnion()
  %12 = call ptr @tspp_string_concat(ptr @2, ptr %11)
  %13 = call ptr @tspp_string_concat(ptr %12, ptr @2)
  call void @tspp_console_log(ptr %13)
  %14 = call i32 @getConstValue()
  %15 = call ptr @tspp_int_to_string(i32 %14)
  call void @tspp_console_log(ptr %15)
  call void @tspp_free_string(ptr %15)
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_bool_to_string(i1)

declare ptr @tspp_string_concat(ptr, ptr)

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_ptr_to_string(ptr)

attributes #0 = { "tspp.const" }

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
