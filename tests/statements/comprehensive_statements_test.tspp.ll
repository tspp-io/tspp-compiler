; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@globalVar = global i32 10
@localVar = global i32 0
@whileCounter = global i32 0
@i = global i32 0
@0 = private unnamed_addr constant [30 x i8] c"\22Testing all statement types\22\00", align 1
@1 = private unnamed_addr constant [18 x i8] c"\22Local variable:\22\00", align 1
@2 = private unnamed_addr constant [31 x i8] c"\22Global var is greater than 5\22\00", align 1
@3 = private unnamed_addr constant [26 x i8] c"\22Global var is 5 or less\22\00", align 1
@4 = private unnamed_addr constant [24 x i8] c"\22While loop iteration:\22\00", align 1
@5 = private unnamed_addr constant [22 x i8] c"\22For loop iteration:\22\00", align 1
@6 = private unnamed_addr constant [22 x i8] c"\22Updated global var:\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  store i32 5, ptr @localVar, align 4
  call void @tspp_console_log(ptr @1)
  %0 = load i32, ptr @localVar, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load i32, ptr @globalVar, align 4
  %cmptmp = icmp sgt i32 %2, 5
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %entry
  call void @tspp_console_log(ptr @2)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  store i32 0, ptr @whileCounter, align 4
  br label %while.cond

else:                                             ; preds = %entry
  call void @tspp_console_log(ptr @3)
  br label %ifcont

while.cond:                                       ; preds = %while.body, %ifcont
  %3 = load i32, ptr @whileCounter, align 4
  %cmptmp1 = icmp slt i32 %3, 2
  br i1 %cmptmp1, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  call void @tspp_console_log(ptr @4)
  %4 = load i32, ptr @whileCounter, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = load i32, ptr @whileCounter, align 4
  %addtmp = add i32 %6, 1
  store i32 %addtmp, ptr @whileCounter, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  store i32 0, ptr @i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %while.end
  %7 = load i32, ptr @i, align 4
  %cmptmp2 = icmp slt i32 %7, 3
  br i1 %cmptmp2, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call void @tspp_console_log(ptr @5)
  %8 = load i32, ptr @i, align 4
  %9 = call ptr @tspp_int_to_string(i32 %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %9)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, ptr @i, align 4
  %addtmp3 = add i32 %10, 1
  store i32 %addtmp3, ptr @i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %11 = load i32, ptr @globalVar, align 4
  %addtmp4 = add i32 %11, 1
  store i32 %addtmp4, ptr @globalVar, align 4
  call void @tspp_console_log(ptr @6)
  %12 = load i32, ptr @globalVar, align 4
  %13 = call ptr @tspp_int_to_string(i32 %12)
  call void @tspp_console_log(ptr %13)
  call void @tspp_free_string(ptr %13)
  ret i1 true
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)
