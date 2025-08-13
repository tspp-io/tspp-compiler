; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@x = global i32 10
@y = global i32 20
@sum = global i32 30
@product = global i32 200
@0 = private constant [6 x i8] c"Hello\00"
@1 = private constant [8 x i8] c"\22Hello\22\00"
@str = global ptr @1
@2 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i1 @main() {
entry:
  %0 = load i32, ptr @sum, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load i32, ptr @product, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = load ptr, ptr @str, align 8
  %5 = call ptr @tspp_string_concat(ptr @2, ptr %4)
  %6 = call ptr @tspp_string_concat(ptr %5, ptr @2)
  call void @tspp_console_log(ptr %6)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_string_concat(ptr, ptr)
