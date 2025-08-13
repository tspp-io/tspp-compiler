; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@x = global i32 42
@y = global i32 23
@z = global i32 65
@0 = private constant [6 x i8] c"entry\00"
@1 = private constant [8 x i8] c"\22entry\22\00"
@s = global ptr @1
@2 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i1 @main() {
entry:
  %0 = load i32, ptr @z, align 4
  %icmpolt = icmp slt i32 %0, 70
  br i1 %icmpolt, label %then, label %ifcont

then:                                             ; preds = %entry
  %1 = load ptr, ptr @s, align 8
  %2 = call ptr @tspp_string_concat(ptr @2, ptr %1)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @2)
  call void @tspp_console_log(ptr %3)
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  %4 = load i32, ptr @z, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  ret i1 false
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
