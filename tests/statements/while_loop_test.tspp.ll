; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@counter = global i32 0
@0 = private unnamed_addr constant [21 x i8] c"\22Testing while loop\22\00", align 1
@1 = private unnamed_addr constant [23 x i8] c"\22Final counter value:\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32, ptr @counter, align 4
  %cmptmp = icmp slt i32 %0, 3
  br i1 %cmptmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %1 = load i32, ptr @counter, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %2)
  %3 = load i32, ptr @counter, align 4
  %addtmp = add i32 %3, 1
  store i32 %addtmp, ptr @counter, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  call void @tspp_console_log(ptr @1)
  %4 = load i32, ptr @counter, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  ret i1 true
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
