; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 42
@0 = private unnamed_addr constant [8 x i8] c"\22entry\22\00", align 1
@s = global ptr @0

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  call void @tspp_console_log(ptr null)
  ret i1 false
}

declare void @tspp_console_log(ptr)
