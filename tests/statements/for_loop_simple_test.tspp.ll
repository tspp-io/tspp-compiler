; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@i = global i32 0
@0 = private unnamed_addr constant [19 x i8] c"\22Testing for loop\22\00", align 1
@1 = private unnamed_addr constant [18 x i8] c"\22Loop iteration:\22\00", align 1
@2 = private unnamed_addr constant [21 x i8] c"\22For loop completed\22\00", align 1
@3 = private unnamed_addr constant [17 x i8] c"\22Final i value:\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  store i32 0, ptr @i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr @i, align 4
  %cmptmp = icmp slt i32 %0, 5
  br i1 %cmptmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call void @tspp_console_log(ptr @1)
  %1 = load i32, ptr @i, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %2)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr @i, align 4
  %addtmp = add i32 %3, 1
  store i32 %addtmp, ptr @i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @tspp_console_log(ptr @2)
  call void @tspp_console_log(ptr @3)
  %4 = load i32, ptr @i, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  ret i1 true
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
