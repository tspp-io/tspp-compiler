; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 10
@y = global i32 20
@sum = global i32 30
@product = global i32 200
@0 = private constant [8 x i8] c"\22Hello\22\00"
@str = global ptr @0

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
  call void @tspp_console_log(ptr %4)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
