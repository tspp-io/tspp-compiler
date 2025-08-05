; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 10
@y = global i32 20
@z = global i32 0
@flag = global i1 false

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  %1 = load i32, ptr @y, align 4
  %addtmp = add i32 %0, %1
  store i32 %addtmp, ptr @z, align 4
  %2 = load i32, ptr @z, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = load i32, ptr @x, align 4
  %5 = load i32, ptr @y, align 4
  %addtmp1 = add i32 %4, %5
  %multmp = mul i32 %addtmp1, 2
  %subtmp = sub i32 %multmp, 5
  store i32 %subtmp, ptr @z, align 4
  %6 = load i32, ptr @z, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  call void @tspp_console_log(ptr %7)
  call void @tspp_free_string(ptr %7)
  %8 = load i32, ptr @x, align 4
  %9 = load i32, ptr @y, align 4
  %cmptmp = icmp slt i32 %8, %9
  store i1 %cmptmp, ptr @flag, align 1
  %10 = load i1, ptr @flag, align 1
  %11 = call ptr @tspp_bool_to_string(i1 %10)
  call void @tspp_console_log(ptr %11)
  call void @tspp_free_string(ptr %11)
  %12 = load i32, ptr @x, align 4
  %cmptmp2 = icmp sgt i32 %12, 5
  br i1 %cmptmp2, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %13 = load i32, ptr @y, align 4
  %cmptmp3 = icmp slt i32 %13, 30
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %cmptmp3, %land.rhs ]
  store i1 %land, ptr @flag, align 1
  %14 = load i1, ptr @flag, align 1
  %15 = call ptr @tspp_bool_to_string(i1 %14)
  call void @tspp_console_log(ptr %15)
  call void @tspp_free_string(ptr %15)
  %16 = load i32, ptr @x, align 4
  %neg = sub i32 0, %16
  store i32 %neg, ptr @z, align 4
  %17 = load i32, ptr @z, align 4
  %18 = call ptr @tspp_int_to_string(i32 %17)
  call void @tspp_console_log(ptr %18)
  call void @tspp_free_string(ptr %18)
  %19 = load i32, ptr @x, align 4
  %addtmp4 = add i32 %19, 5
  store i32 %addtmp4, ptr @x, align 4
  %20 = load i32, ptr @x, align 4
  %21 = call ptr @tspp_int_to_string(i32 %20)
  call void @tspp_console_log(ptr %21)
  call void @tspp_free_string(ptr %21)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_bool_to_string(i1)
