; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 10
@b = global i32 5
@c = global i32 3
@d = global i32 2
@result = global i32 0
@flag = global i1 false
@0 = private unnamed_addr constant [44 x i8] c"=== COMPLEX ARITHMETIC WITH PARENTHESES ===\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@2 = private unnamed_addr constant [36 x i8] c"=== ALL COMPLEX TESTS COMPLETED ===\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i1 @main() {
entry:
  %0 = call ptr @tspp_string_concat(ptr @1, ptr @0)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @1)
  call void @tspp_console_log(ptr %1)
  %2 = load i32, ptr @a, align 4
  %3 = load i32, ptr @b, align 4
  %addtmp = add i32 %2, %3
  %4 = load i32, ptr @c, align 4
  %5 = load i32, ptr @d, align 4
  %subtmp = sub i32 %4, %5
  %multmp = mul i32 %addtmp, %subtmp
  %6 = load i32, ptr @a, align 4
  %7 = load i32, ptr @b, align 4
  %subtmp1 = sub i32 %6, %7
  %8 = load i32, ptr @c, align 4
  %9 = load i32, ptr @d, align 4
  %addtmp2 = add i32 %8, %9
  %multmp3 = mul i32 %subtmp1, %addtmp2
  %addtmp4 = add i32 %multmp, %multmp3
  store i32 %addtmp4, ptr @result, align 4
  %10 = load i32, ptr @result, align 4
  %11 = call ptr @tspp_int_to_string(i32 %10)
  call void @tspp_console_log(ptr %11)
  call void @tspp_free_string(ptr %11)
  %12 = load i32, ptr @a, align 4
  %13 = load i32, ptr @b, align 4
  %14 = load i32, ptr @c, align 4
  %addtmp5 = add i32 %13, %14
  %multmp6 = mul i32 %12, %addtmp5
  %15 = load i32, ptr @d, align 4
  %16 = load i32, ptr @a, align 4
  %multmp7 = mul i32 %15, %16
  %17 = load i32, ptr @b, align 4
  %divtmp = sdiv i32 %multmp7, %17
  %subtmp8 = sub i32 %multmp6, %divtmp
  store i32 %subtmp8, ptr @result, align 4
  %18 = load i32, ptr @result, align 4
  %19 = call ptr @tspp_int_to_string(i32 %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %19)
  %20 = load i32, ptr @a, align 4
  %21 = load i32, ptr @b, align 4
  %addtmp9 = add i32 %20, %21
  %22 = load i32, ptr @c, align 4
  %multmp10 = mul i32 %addtmp9, %22
  %23 = load i32, ptr @d, align 4
  %24 = load i32, ptr @a, align 4
  %25 = load i32, ptr @b, align 4
  %subtmp11 = sub i32 %24, %25
  %multmp12 = mul i32 %23, %subtmp11
  %26 = load i32, ptr @c, align 4
  %27 = load i32, ptr @d, align 4
  %multmp13 = mul i32 %26, %27
  %addtmp14 = add i32 %multmp12, %multmp13
  %subtmp15 = sub i32 %multmp10, %addtmp14
  store i32 %subtmp15, ptr @result, align 4
  %28 = load i32, ptr @result, align 4
  %29 = call ptr @tspp_int_to_string(i32 %28)
  call void @tspp_console_log(ptr %29)
  call void @tspp_free_string(ptr %29)
  %30 = load i32, ptr @a, align 4
  %31 = load i32, ptr @b, align 4
  %addtmp16 = add i32 %30, %31
  %32 = load i32, ptr @c, align 4
  %modtmp = srem i32 %addtmp16, %32
  %33 = load i32, ptr @a, align 4
  %34 = load i32, ptr @b, align 4
  %multmp17 = mul i32 %33, %34
  %35 = load i32, ptr @c, align 4
  %36 = load i32, ptr @d, align 4
  %addtmp18 = add i32 %35, %36
  %modtmp19 = srem i32 %multmp17, %addtmp18
  %addtmp20 = add i32 %modtmp, %modtmp19
  store i32 %addtmp20, ptr @result, align 4
  %37 = load i32, ptr @result, align 4
  %38 = call ptr @tspp_int_to_string(i32 %37)
  call void @tspp_console_log(ptr %38)
  call void @tspp_free_string(ptr %38)
  %39 = load i32, ptr @a, align 4
  %40 = load i32, ptr @b, align 4
  %addtmp21 = add i32 %39, %40
  %neg = sub i32 0, %addtmp21
  %41 = load i32, ptr @c, align 4
  %42 = load i32, ptr @d, align 4
  %subtmp22 = sub i32 %41, %42
  %neg23 = sub i32 0, %subtmp22
  %addtmp24 = add i32 %neg, %neg23
  store i32 %addtmp24, ptr @result, align 4
  %43 = load i32, ptr @result, align 4
  %44 = call ptr @tspp_int_to_string(i32 %43)
  call void @tspp_console_log(ptr %44)
  call void @tspp_free_string(ptr %44)
  %45 = load i32, ptr @a, align 4
  %46 = load i32, ptr @b, align 4
  %47 = load i32, ptr @c, align 4
  %multmp25 = mul i32 %46, %47
  %addtmp26 = add i32 %45, %multmp25
  %48 = load i32, ptr @d, align 4
  %49 = load i32, ptr @a, align 4
  %50 = load i32, ptr @b, align 4
  %subtmp27 = sub i32 %49, %50
  %51 = load i32, ptr @c, align 4
  %addtmp28 = add i32 %subtmp27, %51
  %divtmp29 = sdiv i32 %48, %addtmp28
  %subtmp30 = sub i32 %addtmp26, %divtmp29
  store i32 %subtmp30, ptr @result, align 4
  %52 = load i32, ptr @result, align 4
  %53 = call ptr @tspp_int_to_string(i32 %52)
  call void @tspp_console_log(ptr %53)
  call void @tspp_free_string(ptr %53)
  %54 = load i32, ptr @a, align 4
  %55 = load i32, ptr @b, align 4
  %addtmp31 = add i32 %54, %55
  %56 = load i32, ptr @c, align 4
  %57 = load i32, ptr @d, align 4
  %subtmp32 = sub i32 %56, %57
  %multmp33 = mul i32 %addtmp31, %subtmp32
  %58 = load i32, ptr @a, align 4
  %59 = load i32, ptr @b, align 4
  %subtmp34 = sub i32 %58, %59
  %60 = load i32, ptr @c, align 4
  %addtmp35 = add i32 %subtmp34, %60
  %divtmp36 = sdiv i32 %multmp33, %addtmp35
  store i32 %divtmp36, ptr @result, align 4
  %61 = load i32, ptr @result, align 4
  %62 = call ptr @tspp_int_to_string(i32 %61)
  call void @tspp_console_log(ptr %62)
  call void @tspp_free_string(ptr %62)
  %63 = load i32, ptr @a, align 4
  %64 = load i32, ptr @b, align 4
  %multmp37 = mul i32 %63, %64
  %65 = load i32, ptr @c, align 4
  %66 = load i32, ptr @d, align 4
  %multmp38 = mul i32 %65, %66
  %addtmp39 = add i32 %multmp37, %multmp38
  %67 = load i32, ptr @a, align 4
  %68 = load i32, ptr @c, align 4
  %addtmp40 = add i32 %67, %68
  %69 = load i32, ptr @b, align 4
  %70 = load i32, ptr @d, align 4
  %subtmp41 = sub i32 %69, %70
  %multmp42 = mul i32 %addtmp40, %subtmp41
  %subtmp43 = sub i32 %addtmp39, %multmp42
  %71 = load i32, ptr @a, align 4
  %72 = load i32, ptr @b, align 4
  %subtmp44 = sub i32 %71, %72
  %73 = load i32, ptr @c, align 4
  %74 = load i32, ptr @d, align 4
  %addtmp45 = add i32 %73, %74
  %addtmp46 = add i32 %subtmp44, %addtmp45
  %divtmp47 = sdiv i32 %subtmp43, %addtmp46
  store i32 %divtmp47, ptr @result, align 4
  %75 = load i32, ptr @result, align 4
  %76 = call ptr @tspp_int_to_string(i32 %75)
  call void @tspp_console_log(ptr %76)
  call void @tspp_free_string(ptr %76)
  %77 = load i32, ptr @a, align 4
  %78 = load i32, ptr @b, align 4
  %addtmp48 = add i32 %77, %78
  %79 = load i32, ptr @c, align 4
  %80 = load i32, ptr @d, align 4
  %multmp49 = mul i32 %79, %80
  %icmpogt = icmp sgt i32 %addtmp48, %multmp49
  store i1 %icmpogt, ptr @flag, align 1
  %81 = load i1, ptr @flag, align 1
  %82 = call ptr @tspp_bool_to_string(i1 %81)
  call void @tspp_console_log(ptr %82)
  call void @tspp_free_string(ptr %82)
  %83 = load i32, ptr @a, align 4
  %84 = load i32, ptr @b, align 4
  %subtmp50 = sub i32 %83, %84
  %85 = load i32, ptr @c, align 4
  %86 = load i32, ptr @d, align 4
  %addtmp51 = add i32 %85, %86
  %icmpolt = icmp slt i32 %subtmp50, %addtmp51
  store i1 %icmpolt, ptr @flag, align 1
  %87 = load i1, ptr @flag, align 1
  %88 = call ptr @tspp_bool_to_string(i1 %87)
  call void @tspp_console_log(ptr %88)
  call void @tspp_free_string(ptr %88)
  %89 = load i32, ptr @a, align 4
  %90 = load i32, ptr @b, align 4
  %icmpogt52 = icmp sgt i32 %89, %90
  br i1 %icmpogt52, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %91 = load i32, ptr @c, align 4
  %92 = load i32, ptr @d, align 4
  %icmpogt53 = icmp sgt i32 %91, %92
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %icmpogt53, %land.rhs ]
  store i1 %land, ptr @flag, align 1
  %93 = load i1, ptr @flag, align 1
  %94 = call ptr @tspp_bool_to_string(i1 %93)
  call void @tspp_console_log(ptr %94)
  call void @tspp_free_string(ptr %94)
  %95 = load i32, ptr @a, align 4
  %96 = load i32, ptr @b, align 4
  %icmpolt54 = icmp slt i32 %95, %96
  %not = xor i1 %icmpolt54, true
  store i1 %not, ptr @flag, align 1
  %97 = load i1, ptr @flag, align 1
  %98 = call ptr @tspp_bool_to_string(i1 %97)
  call void @tspp_console_log(ptr %98)
  call void @tspp_free_string(ptr %98)
  %99 = load i32, ptr @a, align 4
  %100 = load i32, ptr @b, align 4
  %addtmp55 = add i32 %99, %100
  %101 = load i32, ptr @c, align 4
  %multmp56 = mul i32 %addtmp55, %101
  %102 = load i32, ptr @a, align 4
  %103 = load i32, ptr @b, align 4
  %subtmp57 = sub i32 %102, %103
  %104 = load i32, ptr @d, align 4
  %multmp58 = mul i32 %subtmp57, %104
  %addtmp59 = add i32 %multmp56, %multmp58
  %105 = load i32, ptr @a, align 4
  %106 = load i32, ptr @b, align 4
  %multmp60 = mul i32 %105, %106
  %107 = load i32, ptr @c, align 4
  %108 = load i32, ptr @d, align 4
  %multmp61 = mul i32 %107, %108
  %subtmp62 = sub i32 %multmp60, %multmp61
  %109 = load i32, ptr @a, align 4
  %110 = load i32, ptr @c, align 4
  %addtmp63 = add i32 %109, %110
  %111 = load i32, ptr @b, align 4
  %112 = load i32, ptr @d, align 4
  %subtmp64 = sub i32 %111, %112
  %subtmp65 = sub i32 %addtmp63, %subtmp64
  %divtmp66 = sdiv i32 %subtmp62, %subtmp65
  %subtmp67 = sub i32 %addtmp59, %divtmp66
  store i32 %subtmp67, ptr @result, align 4
  %113 = load i32, ptr @result, align 4
  %114 = call ptr @tspp_int_to_string(i32 %113)
  call void @tspp_console_log(ptr %114)
  call void @tspp_free_string(ptr %114)
  %115 = load i32, ptr @a, align 4
  %116 = load i32, ptr @b, align 4
  %117 = load i32, ptr @c, align 4
  %118 = load i32, ptr @d, align 4
  %addtmp68 = add i32 %117, %118
  %multmp69 = mul i32 %116, %addtmp68
  %addtmp70 = add i32 %115, %multmp69
  %119 = load i32, ptr @a, align 4
  %120 = load i32, ptr @b, align 4
  %subtmp71 = sub i32 %119, %120
  %121 = load i32, ptr @c, align 4
  %122 = load i32, ptr @d, align 4
  %123 = load i32, ptr @a, align 4
  %addtmp72 = add i32 %122, %123
  %multmp73 = mul i32 %121, %addtmp72
  %addtmp74 = add i32 %subtmp71, %multmp73
  %subtmp75 = sub i32 %addtmp70, %addtmp74
  store i32 %subtmp75, ptr @result, align 4
  %124 = load i32, ptr @result, align 4
  %125 = call ptr @tspp_int_to_string(i32 %124)
  call void @tspp_console_log(ptr %125)
  call void @tspp_free_string(ptr %125)
  %126 = call ptr @tspp_string_concat(ptr @3, ptr @2)
  %127 = call ptr @tspp_string_concat(ptr %126, ptr @3)
  call void @tspp_console_log(ptr %127)
  ret i1 false
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
