; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@basicVar = global i32 10
@stackVar = global i32 20
@heapVar = global i32 30
@0 = private unnamed_addr constant [29 x i8] c"\22Testing storage qualifiers\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
