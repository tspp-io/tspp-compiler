; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 42
@0 = private unnamed_addr constant [17 x i8] c"\22literal string\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  ret i1 false
}

declare void @tspp_console_log(ptr)
