; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 15
@b = global i32 4
@c = global i32 0
@flag1 = global i1 false
@flag2 = global i1 true
@0 = private unnamed_addr constant [21 x i8] c"\22=== ARITHMETIC ===\22\00", align 1
@1 = private unnamed_addr constant [21 x i8] c"\22=== COMPARISON ===\22\00", align 1
@2 = private unnamed_addr constant [16 x i8] c"\22=== UNARY ===\22\00", align 1
@3 = private unnamed_addr constant [21 x i8] c"\22=== PRECEDENCE ===\22\00", align 1

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
  %44 = load i32, ptr @a, align 4
  %neg = sub i32 0, %44
  store i32 %neg, ptr @c, align 4
  %45 = load i32, ptr @c, align 4
  %46 = call ptr @tspp_int_to_string(i32 %45)
  call void @tspp_console_log(ptr %46)
  call void @tspp_free_string(ptr %46)
  %47 = load i1, ptr @flag2, align 1
  %not = xor i1 %47, true
  store i1 %not, ptr @flag1, align 1
  %48 = load i1, ptr @flag1, align 1
  %49 = call ptr @tspp_bool_to_string(i1 %48)
  call void @tspp_console_log(ptr %49)
  call void @tspp_free_string(ptr %49)
  call void @tspp_console_log(ptr @3)
  %50 = load i32, ptr @a, align 4
  %51 = load i32, ptr @b, align 4
  %multmp6 = mul i32 %51, 2
  %addtmp7 = add i32 %50, %multmp6
  store i32 %addtmp7, ptr @c, align 4
  %52 = load i32, ptr @c, align 4
  %53 = call ptr @tspp_int_to_string(i32 %52)
  call void @tspp_console_log(ptr %53)
  call void @tspp_free_string(ptr %53)
  %54 = load i32, ptr @a, align 4
  %55 = load i32, ptr @b, align 4
  %addtmp8 = add i32 %54, %55
  %multmp9 = mul i32 %addtmp8, 2
  store i32 %multmp9, ptr @c, align 4
  %56 = load i32, ptr @c, align 4
  %57 = call ptr @tspp_int_to_string(i32 %56)
  call void @tspp_console_log(ptr %57)
  call void @tspp_free_string(ptr %57)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
