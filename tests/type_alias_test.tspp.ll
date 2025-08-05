; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 0
@b = global i32 0
@c = global i32 0
@d = global i32 0

define i1 @main() {
entry:
  %0 = load i32, ptr @a, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load i32, ptr @d, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
