; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@score = global i32 100
@0 = private constant [33 x i8] c"\22TSPP++ Typedef System Working!\22\00"
@message = global ptr @0
@count = global i32 42
@1 = private constant [12 x i8] c"\22developer\22\00"
@username = global ptr @1

define i1 @main() {
entry:
  %0 = load i32, ptr @score, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load ptr, ptr @message, align 8
  call void @tspp_console_log(ptr %2)
  %3 = load i32, ptr @count, align 4
  %4 = call ptr @tspp_int_to_string(i32 %3)
  call void @tspp_console_log(ptr %4)
  call void @tspp_free_string(ptr %4)
  %5 = load ptr, ptr @username, align 8
  call void @tspp_console_log(ptr %5)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
