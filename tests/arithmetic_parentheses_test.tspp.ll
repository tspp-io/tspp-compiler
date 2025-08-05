; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@a = global i32 10
@b = global i32 5
@c = global i32 3
@d = global i32 2
@result = global i32 0
@flag = global i1 false
@0 = private unnamed_addr constant [46 x i8] c"\22=== COMPLEX ARITHMETIC WITH PARENTHESES ===\22\00", align 1
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
  %modtmp = srem i32 %addtmp16, %30
  %31 = load i32, ptr @a, align 4
  %32 = load i32, ptr @b, align 4
  %multmp17 = mul i32 %31, %32
  %33 = load i32, ptr @c, align 4
  %34 = load i32, ptr @d, align 4
  %addtmp18 = add i32 %33, %34
  %modtmp19 = srem i32 %multmp17, %addtmp18
  %addtmp20 = add i32 %modtmp, %modtmp19
  store i32 %addtmp20, ptr @result, align 4
  %35 = load i32, ptr @result, align 4
  %36 = call ptr @tspp_int_to_string(i32 %35)
  call void @tspp_console_log(ptr %36)
  call void @tspp_free_string(ptr %36)
  %37 = load i32, ptr @a, align 4
  %38 = load i32, ptr @b, align 4
  %addtmp21 = add i32 %37, %38
  %neg = sub i32 0, %addtmp21
  %39 = load i32, ptr @c, align 4
  %40 = load i32, ptr @d, align 4
  %subtmp22 = sub i32 %39, %40
  %neg23 = sub i32 0, %subtmp22
  %addtmp24 = add i32 %neg, %neg23
  store i32 %addtmp24, ptr @result, align 4
  %41 = load i32, ptr @result, align 4
  %42 = call ptr @tspp_int_to_string(i32 %41)
  call void @tspp_console_log(ptr %42)
  call void @tspp_free_string(ptr %42)
  %43 = load i32, ptr @a, align 4
  %44 = load i32, ptr @b, align 4
  %45 = load i32, ptr @c, align 4
  %multmp25 = mul i32 %44, %45
  %addtmp26 = add i32 %43, %multmp25
  %46 = load i32, ptr @d, align 4
  %47 = load i32, ptr @a, align 4
  %48 = load i32, ptr @b, align 4
  %subtmp27 = sub i32 %47, %48
  %49 = load i32, ptr @c, align 4
  %addtmp28 = add i32 %subtmp27, %49
  %divtmp29 = sdiv i32 %46, %addtmp28
  %subtmp30 = sub i32 %addtmp26, %divtmp29
  store i32 %subtmp30, ptr @result, align 4
  %50 = load i32, ptr @result, align 4
  %51 = call ptr @tspp_int_to_string(i32 %50)
  call void @tspp_console_log(ptr %51)
  call void @tspp_free_string(ptr %51)
  %52 = load i32, ptr @a, align 4
  %53 = load i32, ptr @b, align 4
  %addtmp31 = add i32 %52, %53
  %54 = load i32, ptr @c, align 4
  %55 = load i32, ptr @d, align 4
  %subtmp32 = sub i32 %54, %55
  %multmp33 = mul i32 %addtmp31, %subtmp32
  %56 = load i32, ptr @a, align 4
  %57 = load i32, ptr @b, align 4
  %subtmp34 = sub i32 %56, %57
  %58 = load i32, ptr @c, align 4
  %addtmp35 = add i32 %subtmp34, %58
  %divtmp36 = sdiv i32 %multmp33, %addtmp35
  store i32 %divtmp36, ptr @result, align 4
  %59 = load i32, ptr @result, align 4
  %60 = call ptr @tspp_int_to_string(i32 %59)
  call void @tspp_console_log(ptr %60)
  call void @tspp_free_string(ptr %60)
  %61 = load i32, ptr @a, align 4
  %62 = load i32, ptr @b, align 4
  %multmp37 = mul i32 %61, %62
  %63 = load i32, ptr @c, align 4
  %64 = load i32, ptr @d, align 4
  %multmp38 = mul i32 %63, %64
  %addtmp39 = add i32 %multmp37, %multmp38
  %65 = load i32, ptr @a, align 4
  %66 = load i32, ptr @c, align 4
  %addtmp40 = add i32 %65, %66
  %67 = load i32, ptr @b, align 4
  %68 = load i32, ptr @d, align 4
  %subtmp41 = sub i32 %67, %68
  %multmp42 = mul i32 %addtmp40, %subtmp41
  %subtmp43 = sub i32 %addtmp39, %multmp42
  %69 = load i32, ptr @a, align 4
  %70 = load i32, ptr @b, align 4
  %subtmp44 = sub i32 %69, %70
  %71 = load i32, ptr @c, align 4
  %72 = load i32, ptr @d, align 4
  %addtmp45 = add i32 %71, %72
  %addtmp46 = add i32 %subtmp44, %addtmp45
  %divtmp47 = sdiv i32 %subtmp43, %addtmp46
  store i32 %divtmp47, ptr @result, align 4
  %73 = load i32, ptr @result, align 4
  %74 = call ptr @tspp_int_to_string(i32 %73)
  call void @tspp_console_log(ptr %74)
  call void @tspp_free_string(ptr %74)
  %75 = load i32, ptr @a, align 4
  %76 = load i32, ptr @b, align 4
  %addtmp48 = add i32 %75, %76
  %77 = load i32, ptr @c, align 4
  %78 = load i32, ptr @d, align 4
  %multmp49 = mul i32 %77, %78
  %cmptmp = icmp sgt i32 %addtmp48, %multmp49
  store i1 %cmptmp, ptr @flag, align 1
  %79 = load i1, ptr @flag, align 1
  %80 = call ptr @tspp_bool_to_string(i1 %79)
  call void @tspp_console_log(ptr %80)
  call void @tspp_free_string(ptr %80)
  %81 = load i32, ptr @a, align 4
  %82 = load i32, ptr @b, align 4
  %subtmp50 = sub i32 %81, %82
  %83 = load i32, ptr @c, align 4
  %84 = load i32, ptr @d, align 4
  %addtmp51 = add i32 %83, %84
  %cmptmp52 = icmp slt i32 %subtmp50, %addtmp51
  store i1 %cmptmp52, ptr @flag, align 1
  %85 = load i1, ptr @flag, align 1
  %86 = call ptr @tspp_bool_to_string(i1 %85)
  call void @tspp_console_log(ptr %86)
  call void @tspp_free_string(ptr %86)
  %87 = load i32, ptr @a, align 4
  %88 = load i32, ptr @b, align 4
  %cmptmp53 = icmp sgt i32 %87, %88
  br i1 %cmptmp53, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %89 = load i32, ptr @c, align 4
  %90 = load i32, ptr @d, align 4
  %cmptmp54 = icmp sgt i32 %89, %90
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %cmptmp54, %land.rhs ]
  store i1 %land, ptr @flag, align 1
  %91 = load i1, ptr @flag, align 1
  %92 = call ptr @tspp_bool_to_string(i1 %91)
  call void @tspp_console_log(ptr %92)
  call void @tspp_free_string(ptr %92)
  %93 = load i32, ptr @a, align 4
  %94 = load i32, ptr @b, align 4
  %cmptmp55 = icmp slt i32 %93, %94
  %not = xor i1 %cmptmp55, true
  store i1 %not, ptr @flag, align 1
  %95 = load i1, ptr @flag, align 1
  %96 = call ptr @tspp_bool_to_string(i1 %95)
  call void @tspp_console_log(ptr %96)
  call void @tspp_free_string(ptr %96)
  %97 = load i32, ptr @a, align 4
  %98 = load i32, ptr @b, align 4
  %addtmp56 = add i32 %97, %98
  %99 = load i32, ptr @c, align 4
  %multmp57 = mul i32 %addtmp56, %99
  %100 = load i32, ptr @a, align 4
  %101 = load i32, ptr @b, align 4
  %subtmp58 = sub i32 %100, %101
  %102 = load i32, ptr @d, align 4
  %multmp59 = mul i32 %subtmp58, %102
  %addtmp60 = add i32 %multmp57, %multmp59
  %103 = load i32, ptr @a, align 4
  %104 = load i32, ptr @b, align 4
  %multmp61 = mul i32 %103, %104
  %105 = load i32, ptr @c, align 4
  %106 = load i32, ptr @d, align 4
  %multmp62 = mul i32 %105, %106
  %subtmp63 = sub i32 %multmp61, %multmp62
  %107 = load i32, ptr @a, align 4
  %108 = load i32, ptr @c, align 4
  %addtmp64 = add i32 %107, %108
  %109 = load i32, ptr @b, align 4
  %110 = load i32, ptr @d, align 4
  %subtmp65 = sub i32 %109, %110
  %subtmp66 = sub i32 %addtmp64, %subtmp65
  %divtmp67 = sdiv i32 %subtmp63, %subtmp66
  %subtmp68 = sub i32 %addtmp60, %divtmp67
  store i32 %subtmp68, ptr @result, align 4
  %111 = load i32, ptr @result, align 4
  %112 = call ptr @tspp_int_to_string(i32 %111)
  call void @tspp_console_log(ptr %112)
  call void @tspp_free_string(ptr %112)
  %113 = load i32, ptr @a, align 4
  %114 = load i32, ptr @b, align 4
  %115 = load i32, ptr @c, align 4
  %116 = load i32, ptr @d, align 4
  %addtmp69 = add i32 %115, %116
  %multmp70 = mul i32 %114, %addtmp69
  %addtmp71 = add i32 %113, %multmp70
  %117 = load i32, ptr @a, align 4
  %118 = load i32, ptr @b, align 4
  %subtmp72 = sub i32 %117, %118
  %119 = load i32, ptr @c, align 4
  %120 = load i32, ptr @d, align 4
  %121 = load i32, ptr @a, align 4
  %addtmp73 = add i32 %120, %121
  %multmp74 = mul i32 %119, %addtmp73
  %addtmp75 = add i32 %subtmp72, %multmp74
  %subtmp76 = sub i32 %addtmp71, %addtmp75
  store i32 %subtmp76, ptr @result, align 4
  %122 = load i32, ptr @result, align 4
  %123 = call ptr @tspp_int_to_string(i32 %122)
  call void @tspp_console_log(ptr %123)
  call void @tspp_free_string(ptr %123)
  call void @tspp_console_log(ptr @1)
  ret i1 false
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
