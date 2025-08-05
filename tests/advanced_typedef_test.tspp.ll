; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 42
@0 = private constant [19 x i8] c"\22Hello, typedefs!\22\00"
@msg = global ptr @0
@count = global i32 100

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load ptr, ptr @msg, align 8
  call void @tspp_console_log(ptr %2)
  %3 = load i32, ptr @count, align 4
  %4 = call ptr @tspp_int_to_string(i32 %3)
  call void @tspp_console_log(ptr %4)
  call void @tspp_free_string(ptr %4)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
