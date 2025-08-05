; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 15
@b = global i32 4
@c = global i32 0
@flag1 = global i1 false
@flag2 = global i1 true
@0 = private unnamed_addr constant [21 x i8] c"\22=== ARITHMETIC ===\22\00", align 1
@1 = private unnamed_addr constant [21 x i8] c"\22=== COMPARISON ===\22\00", align 1
@2 = private unnamed_addr constant [18 x i8] c"\22=== LOGICAL ===\22\00", align 1
@3 = private unnamed_addr constant [16 x i8] c"\22=== UNARY ===\22\00", align 1
@4 = private unnamed_addr constant [18 x i8] c"\22=== COMPLEX ===\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  %0 = load i32, ptr @a, align 4
  %1 = load i32, ptr @b, align 4
  %addtmp = add i32 %0, %1
  store i32 %addtmp, ptr @c, align 4
  %2 = load i32, ptr @c, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = load i32, ptr @a, align 4
  %5 = load i32, ptr @b, align 4
  %subtmp = sub i32 %4, %5
  store i32 %subtmp, ptr @c, align 4
  %6 = load i32, ptr @c, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  call void @tspp_console_log(ptr %7)
  call void @tspp_free_string(ptr %7)
  %8 = load i32, ptr @a, align 4
  %9 = load i32, ptr @b, align 4
  %multmp = mul i32 %8, %9
  store i32 %multmp, ptr @c, align 4
  %10 = load i32, ptr @c, align 4
  %11 = call ptr @tspp_int_to_string(i32 %10)
  call void @tspp_console_log(ptr %11)
  call void @tspp_free_string(ptr %11)
  %12 = load i32, ptr @a, align 4
  %13 = load i32, ptr @b, align 4
  %divtmp = sdiv i32 %12, %13
  store i32 %divtmp, ptr @c, align 4
  %14 = load i32, ptr @c, align 4
  %15 = call ptr @tspp_int_to_string(i32 %14)
  call void @tspp_console_log(ptr %15)
  call void @tspp_free_string(ptr %15)
  %16 = load i32, ptr @a, align 4
  %17 = load i32, ptr @b, align 4
  %modtmp = srem i32 %16, %17
  store i32 %modtmp, ptr @c, align 4
  %18 = load i32, ptr @c, align 4
  %19 = call ptr @tspp_int_to_string(i32 %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %19)
  call void @tspp_console_log(ptr @1)
  %20 = load i32, ptr @a, align 4
  %21 = load i32, ptr @b, align 4
  %cmptmp = icmp slt i32 %20, %21
  store i1 %cmptmp, ptr @flag1, align 1
  %22 = load i1, ptr @flag1, align 1
  %23 = call ptr @tspp_bool_to_string(i1 %22)
  call void @tspp_console_log(ptr %23)
  call void @tspp_free_string(ptr %23)
  %24 = load i32, ptr @a, align 4
  %25 = load i32, ptr @b, align 4
  %cmptmp1 = icmp sgt i32 %24, %25
  store i1 %cmptmp1, ptr @flag1, align 1
  %26 = load i1, ptr @flag1, align 1
  %27 = call ptr @tspp_bool_to_string(i1 %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %27)
  %28 = load i32, ptr @a, align 4
  %29 = load i32, ptr @b, align 4
  %cmptmp2 = icmp sle i32 %28, %29
  store i1 %cmptmp2, ptr @flag1, align 1
  %30 = load i1, ptr @flag1, align 1
  %31 = call ptr @tspp_bool_to_string(i1 %30)
  call void @tspp_console_log(ptr %31)
  call void @tspp_free_string(ptr %31)
  %32 = load i32, ptr @a, align 4
  %33 = load i32, ptr @b, align 4
  %cmptmp3 = icmp sge i32 %32, %33
  store i1 %cmptmp3, ptr @flag1, align 1
  %34 = load i1, ptr @flag1, align 1
  %35 = call ptr @tspp_bool_to_string(i1 %34)
  call void @tspp_console_log(ptr %35)
  call void @tspp_free_string(ptr %35)
  %36 = load i32, ptr @a, align 4
  %37 = load i32, ptr @b, align 4
  %cmptmp4 = icmp eq i32 %36, %37
  store i1 %cmptmp4, ptr @flag1, align 1
  %38 = load i1, ptr @flag1, align 1
  %39 = call ptr @tspp_bool_to_string(i1 %38)
  call void @tspp_console_log(ptr %39)
  call void @tspp_free_string(ptr %39)
  %40 = load i32, ptr @a, align 4
  %41 = load i32, ptr @b, align 4
  %cmptmp5 = icmp ne i32 %40, %41
  store i1 %cmptmp5, ptr @flag1, align 1
  %42 = load i1, ptr @flag1, align 1
  %43 = call ptr @tspp_bool_to_string(i1 %42)
  call void @tspp_console_log(ptr %43)
  call void @tspp_free_string(ptr %43)
  call void @tspp_console_log(ptr @2)
  %44 = load i1, ptr @flag2, align 1
  br i1 %44, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %45 = load i32, ptr @a, align 4
  %46 = load i32, ptr @b, align 4
  %cmptmp6 = icmp sgt i32 %45, %46
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %cmptmp6, %land.rhs ]
  store i1 %land, ptr @flag1, align 1
  %47 = load i1, ptr @flag1, align 1
  %48 = call ptr @tspp_bool_to_string(i1 %47)
  call void @tspp_console_log(ptr %48)
  call void @tspp_free_string(ptr %48)
  %49 = load i1, ptr @flag1, align 1
  br i1 %49, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %land.end
  %50 = load i32, ptr @a, align 4
  %51 = load i32, ptr @b, align 4
  %cmptmp7 = icmp slt i32 %50, %51
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.end
  %lor = phi i1 [ true, %land.end ], [ %cmptmp7, %lor.rhs ]
  store i1 %lor, ptr @flag1, align 1
  %52 = load i1, ptr @flag1, align 1
  %53 = call ptr @tspp_bool_to_string(i1 %52)
  call void @tspp_console_log(ptr %53)
  call void @tspp_free_string(ptr %53)
  call void @tspp_console_log(ptr @3)
  %54 = load i32, ptr @a, align 4
  %neg = sub i32 0, %54
  store i32 %neg, ptr @c, align 4
  %55 = load i32, ptr @c, align 4
  %56 = call ptr @tspp_int_to_string(i32 %55)
  call void @tspp_console_log(ptr %56)
  call void @tspp_free_string(ptr %56)
  %57 = load i1, ptr @flag2, align 1
  %not = xor i1 %57, true
  store i1 %not, ptr @flag1, align 1
  %58 = load i1, ptr @flag1, align 1
  %59 = call ptr @tspp_bool_to_string(i1 %58)
  call void @tspp_console_log(ptr %59)
  call void @tspp_free_string(ptr %59)
  call void @tspp_console_log(ptr @4)
  %60 = load i32, ptr @a, align 4
  %61 = load i32, ptr @b, align 4
  %multmp8 = mul i32 %61, 2
  %addtmp9 = add i32 %60, %multmp8
  store i32 %addtmp9, ptr @c, align 4
  %62 = load i32, ptr @c, align 4
  %63 = call ptr @tspp_int_to_string(i32 %62)
  call void @tspp_console_log(ptr %63)
  call void @tspp_free_string(ptr %63)
  %64 = load i32, ptr @a, align 4
  %65 = load i32, ptr @b, align 4
  %addtmp10 = add i32 %64, %65
  %multmp11 = mul i32 %addtmp10, 2
  store i32 %multmp11, ptr @c, align 4
  %66 = load i32, ptr @c, align 4
  %67 = call ptr @tspp_int_to_string(i32 %66)
  call void @tspp_console_log(ptr %67)
  call void @tspp_free_string(ptr %67)
  %68 = load i32, ptr @a, align 4
  %cmptmp12 = icmp sgt i32 %68, 10
  br i1 %cmptmp12, label %land.rhs13, label %land.end14

land.rhs13:                                       ; preds = %lor.end
  %69 = load i32, ptr @b, align 4
  %cmptmp15 = icmp slt i32 %69, 10
  br label %land.end14

land.end14:                                       ; preds = %land.rhs13, %lor.end
  %land16 = phi i1 [ false, %lor.end ], [ %cmptmp15, %land.rhs13 ]
  br i1 %land16, label %lor.end18, label %lor.rhs17

lor.rhs17:                                        ; preds = %land.end14
  %70 = load i32, ptr @a, align 4
  %cmptmp19 = icmp eq i32 %70, 15
  br label %lor.end18

lor.end18:                                        ; preds = %lor.rhs17, %land.end14
  %lor20 = phi i1 [ true, %land.end14 ], [ %cmptmp19, %lor.rhs17 ]
  store i1 %lor20, ptr @flag1, align 1
  %71 = load i1, ptr @flag1, align 1
  %72 = call ptr @tspp_bool_to_string(i1 %71)
  call void @tspp_console_log(ptr %72)
  call void @tspp_free_string(ptr %72)
  %73 = load i32, ptr @b, align 4
  %addtmp21 = add i32 %73, 1
  store i32 %addtmp21, ptr @a, align 4
  %74 = load i32, ptr @a, align 4
  %75 = call ptr @tspp_int_to_string(i32 %74)
  call void @tspp_console_log(ptr %75)
  call void @tspp_free_string(ptr %75)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
