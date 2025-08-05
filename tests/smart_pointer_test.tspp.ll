; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@sharedVar = global i32 0
@uniqueVar = global i32 0
@weakVar = global i32 0
@0 = private unnamed_addr constant [31 x i8] c"\22Testing smart pointer syntax\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
