; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 100
@y = global i32 50
@z = global i32 0

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  %1 = load i32, ptr @y, align 4
  %subtmp = sub i32 %0, %1
  store i32 %subtmp, ptr @z, align 4
  %2 = load i32, ptr @z, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
