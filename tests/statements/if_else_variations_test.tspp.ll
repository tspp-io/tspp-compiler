; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 5
@y = global i32 10
@0 = private unnamed_addr constant [29 x i8] c"\22Testing if-else statements\22\00", align 1
@1 = private unnamed_addr constant [16 x i8] c"\22x is positive\22\00", align 1
@2 = private unnamed_addr constant [22 x i8] c"\22x is greater than y\22\00", align 1
@3 = private unnamed_addr constant [26 x i8] c"\22x is not greater than y\22\00", align 1
@4 = private unnamed_addr constant [28 x i8] c"\22Both x and y are positive\22\00", align 1
@5 = private unnamed_addr constant [29 x i8] c"\22x is positive but y is not\22\00", align 1
@6 = private unnamed_addr constant [20 x i8] c"\22x is not positive\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  %0 = load i32, ptr @x, align 4
  %cmptmp = icmp sgt i32 %0, 0
  br i1 %cmptmp, label %then, label %ifcont

then:                                             ; preds = %entry
  call void @tspp_console_log(ptr @1)
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  %1 = load i32, ptr @x, align 4
  %2 = load i32, ptr @y, align 4
  %cmptmp1 = icmp sgt i32 %1, %2
  br i1 %cmptmp1, label %then2, label %else

then2:                                            ; preds = %ifcont
  call void @tspp_console_log(ptr @2)
  br label %ifcont3

ifcont3:                                          ; preds = %else, %then2
  %3 = load i32, ptr @x, align 4
  %cmptmp4 = icmp sgt i32 %3, 0
  br i1 %cmptmp4, label %then5, label %else7

else:                                             ; preds = %ifcont
  call void @tspp_console_log(ptr @3)
  br label %ifcont3

then5:                                            ; preds = %ifcont3
  %4 = load i32, ptr @y, align 4
  %cmptmp8 = icmp sgt i32 %4, 0
  br i1 %cmptmp8, label %then9, label %else11

ifcont6:                                          ; preds = %else7
  ret i1 true

else7:                                            ; preds = %ifcont3
  call void @tspp_console_log(ptr @6)
  br label %ifcont6

then9:                                            ; preds = %then5
  call void @tspp_console_log(ptr @4)
  br label %ifcont10

ifcont10:                                         ; preds = %else11, %then9

else11:                                           ; preds = %then5
  call void @tspp_console_log(ptr @5)
  br label %ifcont10
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
