; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 100
@y = global i32 50
@result = global i32 0
@0 = private unnamed_addr constant [8 x i8] c"\22small\22\00", align 1
@1 = private unnamed_addr constant [8 x i8] c"\22large\22\00", align 1

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  %1 = load i32, ptr @y, align 4
  %subtmp = sub i32 %0, %1
  store i32 %subtmp, ptr @result, align 4
  %2 = load i32, ptr @result, align 4
  %cmptmp = icmp slt i32 %2, 30
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %entry
  call void @tspp_console_log(ptr @0)
  br label %ifcont

else:                                             ; preds = %entry
  call void @tspp_console_log(ptr @1)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %3 = load i32, ptr @result, align 4
  %4 = call ptr @tspp_int_to_string(i32 %3)
  call void @tspp_console_log(ptr %4)
  call void @tspp_free_string(ptr %4)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
