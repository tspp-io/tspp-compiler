; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 5
@y = global i32 10
@0 = private unnamed_addr constant [20 x i8] c"\22Testing nested if\22\00", align 1
@1 = private unnamed_addr constant [16 x i8] c"\22x is positive\22\00", align 1
@2 = private unnamed_addr constant [21 x i8] c"\22y is also positive\22\00", align 1
@3 = private unnamed_addr constant [7 x i8] c"\22Done\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  %0 = load i32, ptr @x, align 4
  %cmptmp = icmp sgt i32 %0, 0
  br i1 %cmptmp, label %then, label %ifcont

then:                                             ; preds = %entry
  call void @tspp_console_log(ptr @1)
  %1 = load i32, ptr @y, align 4
  %cmptmp1 = icmp sgt i32 %1, 0
  br i1 %cmptmp1, label %then2, label %ifcont3

ifcont:                                           ; preds = %entry
  call void @tspp_console_log(ptr @3)
  ret i1 true

then2:                                            ; preds = %then
  call void @tspp_console_log(ptr @2)
  br label %ifcont3

ifcont3:                                          ; preds = %then2, %then
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
