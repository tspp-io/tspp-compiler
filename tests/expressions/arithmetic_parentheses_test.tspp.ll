; ModuleID = 'tspp_module'
source_filename = "tspp_module"
target triple = "x86_64-pc-linux-gnu"

@llvm.global_ctors = appending constant [1 x { i32, ptr }] [{ i32, ptr } { i32 65535, ptr @__tspp_gc_ctor }]
@a = global i32 10
@b = global i32 5
@c = global i32 3
@d = global i32 2
@result = global i32 0
@flag = global i32 false
@0 = private constant [46 x i8] c"\22=== COMPLEX ARITHMETIC WITH PARENTHESES ===\22\00"
@1 = private constant [38 x i8] c"\22=== ALL COMPLEX TESTS COMPLETED ===\22\00"

declare void @GC_INIT()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_INIT()
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
