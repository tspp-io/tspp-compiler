; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 10
@b = global i32 5
@c = global i32 3
@d = global i32 2
@result = global i32 0
@flag = global i1 false
@0 = private unnamed_addr constant [36 x i8] c"\22=== COMPLEX PARENTHESES TESTS ===\22\00", align 1
@1 = private unnamed_addr constant [38 x i8] c"\22=== ALL COMPLEX TESTS COMPLETED ===\22\00", align 1

define i1 @main() {
entry:
  call void @tspp_console_log(ptr @0)
  %0 = load i32, ptr @a, align 4
  %1 = load i32, ptr @b, align 4
  %addtmp = add i32 %0, %1
  %2 = load i32, ptr @c, align 4
  %3 = load i32, ptr @d, align 4
  %subtmp = sub i32 %2, %3
  %multmp = mul i32 %addtmp, %subtmp
  %4 = load i32, ptr @a, align 4
  %5 = load i32, ptr @b, align 4
  %subtmp1 = sub i32 %4, %5
  %6 = load i32, ptr @c, align 4
  %7 = load i32, ptr @d, align 4
  %addtmp2 = add i32 %6, %7
  %multmp3 = mul i32 %subtmp1, %addtmp2
  %addtmp4 = add i32 %multmp, %multmp3
  store i32 %addtmp4, ptr @result, align 4
  %8 = load i32, ptr @result, align 4
  %9 = call ptr @tspp_int_to_string(i32 %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %9)
  %10 = load i32, ptr @a, align 4
  %11 = load i32, ptr @b, align 4
  %12 = load i32, ptr @c, align 4
  %addtmp5 = add i32 %11, %12
  %multmp6 = mul i32 %10, %addtmp5
  %13 = load i32, ptr @d, align 4
  %14 = load i32, ptr @a, align 4
  %multmp7 = mul i32 %13, %14
  %15 = load i32, ptr @b, align 4
  %divtmp = sdiv i32 %multmp7, %15
  %subtmp8 = sub i32 %multmp6, %divtmp
  store i32 %subtmp8, ptr @result, align 4
  %16 = load i32, ptr @result, align 4
  %17 = call ptr @tspp_int_to_string(i32 %16)
  call void @tspp_console_log(ptr %17)
  call void @tspp_free_string(ptr %17)
  %18 = load i32, ptr @a, align 4
  %19 = load i32, ptr @b, align 4
  %addtmp9 = add i32 %18, %19
  %20 = load i32, ptr @c, align 4
  %multmp10 = mul i32 %addtmp9, %20
  %21 = load i32, ptr @d, align 4
  %22 = load i32, ptr @a, align 4
  %23 = load i32, ptr @b, align 4
  %subtmp11 = sub i32 %22, %23
  %multmp12 = mul i32 %21, %subtmp11
  %24 = load i32, ptr @c, align 4
  %25 = load i32, ptr @d, align 4
  %multmp13 = mul i32 %24, %25
  %addtmp14 = add i32 %multmp12, %multmp13
  %subtmp15 = sub i32 %multmp10, %addtmp14
  store i32 %subtmp15, ptr @result, align 4
  %26 = load i32, ptr @result, align 4
  %27 = call ptr @tspp_int_to_string(i32 %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %27)
  %28 = load i32, ptr @a, align 4
  %29 = load i32, ptr @b, align 4
  %addtmp16 = add i32 %28, %29
  %30 = load i32, ptr @c, align 4
  %31 = load i32, ptr @d, align 4
  %multmp17 = mul i32 %30, %31
  %cmptmp = icmp sgt i32 %addtmp16, %multmp17
  br i1 %cmptmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %32 = load i32, ptr @a, align 4
  %33 = load i32, ptr @b, align 4
  %subtmp18 = sub i32 %32, %33
  %34 = load i32, ptr @c, align 4
  %35 = load i32, ptr @d, align 4
  %addtmp19 = add i32 %34, %35
  %cmptmp20 = icmp slt i32 %subtmp18, %addtmp19
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %cmptmp20, %land.rhs ]
  store i1 %land, ptr @flag, align 1
  %36 = load i1, ptr @flag, align 1
  %37 = call ptr @tspp_bool_to_string(i1 %36)
  call void @tspp_console_log(ptr %37)
  call void @tspp_free_string(ptr %37)
  %38 = load i32, ptr @a, align 4
  %39 = load i32, ptr @b, align 4
  %cmptmp21 = icmp sgt i32 %38, %39
  br i1 %cmptmp21, label %land.rhs22, label %land.end23

land.rhs22:                                       ; preds = %land.end
  %40 = load i32, ptr @c, align 4
  %41 = load i32, ptr @d, align 4
  %cmptmp24 = icmp sgt i32 %40, %41
  br i1 %cmptmp24, label %lor.end, label %lor.rhs

land.end23:                                       ; preds = %lor.end, %land.end
  %land28 = phi i1 [ false, %land.end ], [ %lor, %lor.end ]
  store i1 %land28, ptr @flag, align 1
  %42 = load i1, ptr @flag, align 1
  %43 = call ptr @tspp_bool_to_string(i1 %42)
  call void @tspp_console_log(ptr %43)
  call void @tspp_free_string(ptr %43)
  %44 = load i32, ptr @a, align 4
  %45 = load i32, ptr @b, align 4
  %addtmp29 = add i32 %44, %45
  %46 = load i32, ptr @c, align 4
  %modtmp = srem i32 %addtmp29, %46
  %47 = load i32, ptr @a, align 4
  %48 = load i32, ptr @b, align 4
  %multmp30 = mul i32 %47, %48
  %49 = load i32, ptr @c, align 4
  %50 = load i32, ptr @d, align 4
  %addtmp31 = add i32 %49, %50
  %modtmp32 = srem i32 %multmp30, %addtmp31
  %addtmp33 = add i32 %modtmp, %modtmp32
  store i32 %addtmp33, ptr @result, align 4
  %51 = load i32, ptr @result, align 4
  %52 = call ptr @tspp_int_to_string(i32 %51)
  call void @tspp_console_log(ptr %52)
  call void @tspp_free_string(ptr %52)
  %53 = load i32, ptr @a, align 4
  %54 = load i32, ptr @b, align 4
  %addtmp34 = add i32 %53, %54
  %neg = sub i32 0, %addtmp34
  %55 = load i32, ptr @c, align 4
  %56 = load i32, ptr @d, align 4
  %subtmp35 = sub i32 %55, %56
  %neg36 = sub i32 0, %subtmp35
  %addtmp37 = add i32 %neg, %neg36
  store i32 %addtmp37, ptr @result, align 4
  %57 = load i32, ptr @result, align 4
  %58 = call ptr @tspp_int_to_string(i32 %57)
  call void @tspp_console_log(ptr %58)
  call void @tspp_free_string(ptr %58)
  %59 = load i32, ptr @a, align 4
  %60 = load i32, ptr @b, align 4
  %61 = load i32, ptr @c, align 4
  %multmp38 = mul i32 %60, %61
  %addtmp39 = add i32 %59, %multmp38
  %62 = load i32, ptr @d, align 4
  %63 = load i32, ptr @a, align 4
  %64 = load i32, ptr @b, align 4
  %subtmp40 = sub i32 %63, %64
  %65 = load i32, ptr @c, align 4
  %addtmp41 = add i32 %subtmp40, %65
  %divtmp42 = sdiv i32 %62, %addtmp41
  %subtmp43 = sub i32 %addtmp39, %divtmp42
  store i32 %subtmp43, ptr @result, align 4
  %66 = load i32, ptr @result, align 4
  %67 = call ptr @tspp_int_to_string(i32 %66)
  call void @tspp_console_log(ptr %67)
  call void @tspp_free_string(ptr %67)
  %68 = load i32, ptr @a, align 4
  %69 = load i32, ptr @b, align 4
  %addtmp44 = add i32 %68, %69
  %70 = load i32, ptr @c, align 4
  %71 = load i32, ptr @d, align 4
  %subtmp45 = sub i32 %70, %71
  %multmp46 = mul i32 %addtmp44, %subtmp45
  %72 = load i32, ptr @a, align 4
  %73 = load i32, ptr @b, align 4
  %subtmp47 = sub i32 %72, %73
  %74 = load i32, ptr @c, align 4
  %addtmp48 = add i32 %subtmp47, %74
  %divtmp49 = sdiv i32 %multmp46, %addtmp48
  store i32 %divtmp49, ptr @result, align 4
  %75 = load i32, ptr @result, align 4
  %76 = call ptr @tspp_int_to_string(i32 %75)
  call void @tspp_console_log(ptr %76)
  call void @tspp_free_string(ptr %76)
  %77 = load i32, ptr @a, align 4
  %78 = load i32, ptr @b, align 4
  %multmp50 = mul i32 %77, %78
  %79 = load i32, ptr @c, align 4
  %80 = load i32, ptr @d, align 4
  %multmp51 = mul i32 %79, %80
  %addtmp52 = add i32 %multmp50, %multmp51
  %81 = load i32, ptr @a, align 4
  %82 = load i32, ptr @c, align 4
  %addtmp53 = add i32 %81, %82
  %83 = load i32, ptr @b, align 4
  %84 = load i32, ptr @d, align 4
  %subtmp54 = sub i32 %83, %84
  %multmp55 = mul i32 %addtmp53, %subtmp54
  %subtmp56 = sub i32 %addtmp52, %multmp55
  %85 = load i32, ptr @a, align 4
  %86 = load i32, ptr @b, align 4
  %subtmp57 = sub i32 %85, %86
  %87 = load i32, ptr @c, align 4
  %88 = load i32, ptr @d, align 4
  %addtmp58 = add i32 %87, %88
  %addtmp59 = add i32 %subtmp57, %addtmp58
  %divtmp60 = sdiv i32 %subtmp56, %addtmp59
  store i32 %divtmp60, ptr @result, align 4
  %89 = load i32, ptr @result, align 4
  %90 = call ptr @tspp_int_to_string(i32 %89)
  call void @tspp_console_log(ptr %90)
  call void @tspp_free_string(ptr %90)
  %91 = load i32, ptr @a, align 4
  %92 = load i32, ptr @b, align 4
  %cmptmp61 = icmp sgt i32 %91, %92
  br i1 %cmptmp61, label %land.rhs62, label %land.end63

lor.rhs:                                          ; preds = %land.rhs22
  %93 = load i32, ptr @a, align 4
  %94 = load i32, ptr @c, align 4
  %addtmp25 = add i32 %93, %94
  %95 = load i32, ptr @b, align 4
  %96 = load i32, ptr @d, align 4
  %addtmp26 = add i32 %95, %96
  %cmptmp27 = icmp slt i32 %addtmp25, %addtmp26
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.rhs22
  %lor = phi i1 [ true, %land.rhs22 ], [ %cmptmp27, %lor.rhs ]
  br label %land.end23

land.rhs62:                                       ; preds = %land.end23
  %97 = load i32, ptr @c, align 4
  %98 = load i32, ptr @d, align 4
  %cmptmp64 = icmp sgt i32 %97, %98
  br label %land.end63

land.end63:                                       ; preds = %land.rhs62, %land.end23
  %land65 = phi i1 [ false, %land.end23 ], [ %cmptmp64, %land.rhs62 ]
  br i1 %land65, label %lor.end67, label %lor.rhs66

lor.rhs66:                                        ; preds = %land.end63
  %99 = load i32, ptr @a, align 4
  %100 = load i32, ptr @b, align 4
  %cmptmp68 = icmp slt i32 %99, %100
  br i1 %cmptmp68, label %land.rhs69, label %land.end70

lor.end67:                                        ; preds = %land.end70, %land.end63
  %lor73 = phi i1 [ true, %land.end63 ], [ %land72, %land.end70 ]
  store i1 %lor73, ptr @flag, align 1
  %101 = load i1, ptr @flag, align 1
  %102 = call ptr @tspp_bool_to_string(i1 %101)
  call void @tspp_console_log(ptr %102)
  call void @tspp_free_string(ptr %102)
  %103 = load i32, ptr @a, align 4
  %104 = load i32, ptr @b, align 4
  %cmptmp74 = icmp slt i32 %103, %104
  br i1 %cmptmp74, label %lor.end76, label %lor.rhs75

land.rhs69:                                       ; preds = %lor.rhs66
  %105 = load i32, ptr @c, align 4
  %106 = load i32, ptr @d, align 4
  %cmptmp71 = icmp slt i32 %105, %106
  br label %land.end70

land.end70:                                       ; preds = %land.rhs69, %lor.rhs66
  %land72 = phi i1 [ false, %lor.rhs66 ], [ %cmptmp71, %land.rhs69 ]
  br label %lor.end67

lor.rhs75:                                        ; preds = %lor.end67
  %107 = load i32, ptr @c, align 4
  %108 = load i32, ptr @d, align 4
  %cmptmp77 = icmp slt i32 %107, %108
  br label %lor.end76

lor.end76:                                        ; preds = %lor.rhs75, %lor.end67
  %lor78 = phi i1 [ true, %lor.end67 ], [ %cmptmp77, %lor.rhs75 ]
  %not = xor i1 %lor78, true
  store i1 %not, ptr @flag, align 1
  %109 = load i1, ptr @flag, align 1
  %110 = call ptr @tspp_bool_to_string(i1 %109)
  call void @tspp_console_log(ptr %110)
  call void @tspp_free_string(ptr %110)
  call void @tspp_console_log(ptr @1)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
