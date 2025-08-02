; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 42
@y = global i32 23
@z = global i32 0
@s = global ptr @0
@0 = private constant [8 x i8] c"\22entry\22\00"

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  %1 = load i32, ptr @y, align 4
  %addtmp = add i32 %0, %1
  store i32 %addtmp, ptr @z, align 4
  %2 = load i32, ptr @z, align 4
  %cmptmp = icmp slt i32 %2, 70
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %entry
  %3 = load ptr, ptr @s, align 8
  call void @tspp_console_log(ptr %3)
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %4 = load i32, ptr @z, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
