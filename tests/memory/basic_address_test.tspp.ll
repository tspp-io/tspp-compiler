; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 42
@pa = global ptr @a
@0 = private unnamed_addr constant [32 x i8] c"\22Testing @ address-of operator\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  %0 = load i32, ptr @a, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load ptr, ptr @pa, align 8
  %deref = load i32, ptr %2, align 4
  %3 = call ptr @tspp_int_to_string(i32 %deref)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  ret i1 true
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
