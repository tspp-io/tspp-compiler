; ModuleID = 'tspp_module'
source_filename = "tspp_module"
target triple = "x86_64-pc-linux-gnu"

@stackVar = global i32 10
@0 = private constant [8 x i8] c"\22Hello\22\00"
@heapVar = global ptr @0
@staticVar = global i1 true
@basicPtr = global i32 0
@1 = private constant [35 x i8] c"\22Testing memory and type features\22\00"

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
