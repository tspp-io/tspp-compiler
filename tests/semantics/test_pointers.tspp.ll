; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 10
@pa = global i32 0
@val = global i32 0
@b = global float 0x40091EB860000000
@pb = global i32 0
@u1 = global i32 0
@s1 = global i32 0
@s2 = global i32 0
@w1 = global i32 0
@arr = global i32 0
@p = global i32 0
@u = global i32 0
@data = global float 0x4005AE1480000000
@aligned_ptr = global i32 0
@0 = private unnamed_addr constant [28 x i8] c"\22Testing pointer semantics\22\00", align 1

define i1 @testPointerSemantics() {
entry:
  call void @tspp_console_log(ptr @0)
  %0 = load i32, ptr @pa, align 4
  call void @tspp_console_log()
  %1 = load i32, ptr @pb, align 4
  call void @tspp_console_log()
  %2 = load i32, ptr @s1, align 4
  call void @tspp_console_log()
  %3 = load i32, ptr @s2, align 4
  call void @tspp_console_log()
  ret i1 true
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

define i1 @main() {
entry:
  ret void
}
