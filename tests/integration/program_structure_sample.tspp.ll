; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 42
@y = global i32 23
@z = global i32 65
@0 = private constant [8 x i8] c"\22entry\22\00"
@s = global ptr @0

define i1 @main() {
entry:
  %0 = load i32, ptr @z, align 4
  %cmptmp = icmp slt i32 %0, 70
  br i1 %cmptmp, label %then, label %ifcont

then:                                             ; preds = %entry
  %1 = load ptr, ptr @s, align 8
  call void @tspp_console_log(ptr %1)
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  %2 = load i32, ptr @z, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
