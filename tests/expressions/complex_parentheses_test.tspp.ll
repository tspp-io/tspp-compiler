; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 10
@b = global i32 5
@c = global i32 3
@d = global i32 2
@result = global i32 0
@flag = global i1 false
@0 = private unnamed_addr constant [34 x i8] c"=== COMPLEX PARENTHESES TESTS ===\00", align 1
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
  %33 = load i32, ptr @d, align 4
  %multmp17 = mul i32 %32, %33
  %icmpogt = icmp sgt i32 %addtmp16, %multmp17
  br i1 %icmpogt, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %34 = load i32, ptr @a, align 4
  %35 = load i32, ptr @b, align 4
  %subtmp18 = sub i32 %34, %35
  %36 = load i32, ptr @c, align 4
  %37 = load i32, ptr @d, align 4
  %addtmp19 = add i32 %36, %37
  %icmpolt = icmp slt i32 %subtmp18, %addtmp19
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %icmpolt, %land.rhs ]
  store i1 %land, ptr @flag, align 1
  %38 = load i1, ptr @flag, align 1
  %39 = call ptr @tspp_bool_to_string(i1 %38)
  call void @tspp_console_log(ptr %39)
  call void @tspp_free_string(ptr %39)
  %40 = load i32, ptr @a, align 4
  %41 = load i32, ptr @b, align 4
  %icmpogt20 = icmp sgt i32 %40, %41
  br i1 %icmpogt20, label %land.rhs21, label %land.end22

land.rhs21:                                       ; preds = %land.end
  %42 = load i32, ptr @c, align 4
  %43 = load i32, ptr @d, align 4
  %icmpogt23 = icmp sgt i32 %42, %43
  br i1 %icmpogt23, label %lor.end, label %lor.rhs

land.end22:                                       ; preds = %lor.end, %land.end
  %land27 = phi i1 [ false, %land.end ], [ %lor, %lor.end ]
  store i1 %land27, ptr @flag, align 1
  %44 = load i1, ptr @flag, align 1
  %45 = call ptr @tspp_bool_to_string(i1 %44)
  call void @tspp_console_log(ptr %45)
  call void @tspp_free_string(ptr %45)
  %46 = load i32, ptr @a, align 4
  %47 = load i32, ptr @b, align 4
  %addtmp28 = add i32 %46, %47
  %48 = load i32, ptr @c, align 4
  %modtmp = srem i32 %addtmp28, %48
  %49 = load i32, ptr @a, align 4
  %50 = load i32, ptr @b, align 4
  %multmp29 = mul i32 %49, %50
  %51 = load i32, ptr @c, align 4
  %52 = load i32, ptr @d, align 4
  %addtmp30 = add i32 %51, %52
  %modtmp31 = srem i32 %multmp29, %addtmp30
  %addtmp32 = add i32 %modtmp, %modtmp31
  store i32 %addtmp32, ptr @result, align 4
  %53 = load i32, ptr @result, align 4
  %54 = call ptr @tspp_int_to_string(i32 %53)
  call void @tspp_console_log(ptr %54)
  call void @tspp_free_string(ptr %54)
  %55 = load i32, ptr @a, align 4
  %56 = load i32, ptr @b, align 4
  %addtmp33 = add i32 %55, %56
  %neg = sub i32 0, %addtmp33
  %57 = load i32, ptr @c, align 4
  %58 = load i32, ptr @d, align 4
  %subtmp34 = sub i32 %57, %58
  %neg35 = sub i32 0, %subtmp34
  %addtmp36 = add i32 %neg, %neg35
  store i32 %addtmp36, ptr @result, align 4
  %59 = load i32, ptr @result, align 4
  %60 = call ptr @tspp_int_to_string(i32 %59)
  call void @tspp_console_log(ptr %60)
  call void @tspp_free_string(ptr %60)
  %61 = load i32, ptr @a, align 4
  %62 = load i32, ptr @b, align 4
  %63 = load i32, ptr @c, align 4
  %multmp37 = mul i32 %62, %63
  %addtmp38 = add i32 %61, %multmp37
  %64 = load i32, ptr @d, align 4
  %65 = load i32, ptr @a, align 4
  %66 = load i32, ptr @b, align 4
  %subtmp39 = sub i32 %65, %66
  %67 = load i32, ptr @c, align 4
  %addtmp40 = add i32 %subtmp39, %67
  %divtmp41 = sdiv i32 %64, %addtmp40
  %subtmp42 = sub i32 %addtmp38, %divtmp41
  store i32 %subtmp42, ptr @result, align 4
  %68 = load i32, ptr @result, align 4
  %69 = call ptr @tspp_int_to_string(i32 %68)
  call void @tspp_console_log(ptr %69)
  call void @tspp_free_string(ptr %69)
  %70 = load i32, ptr @a, align 4
  %71 = load i32, ptr @b, align 4
  %addtmp43 = add i32 %70, %71
  %72 = load i32, ptr @c, align 4
  %73 = load i32, ptr @d, align 4
  %subtmp44 = sub i32 %72, %73
  %multmp45 = mul i32 %addtmp43, %subtmp44
  %74 = load i32, ptr @a, align 4
  %75 = load i32, ptr @b, align 4
  %subtmp46 = sub i32 %74, %75
  %76 = load i32, ptr @c, align 4
  %addtmp47 = add i32 %subtmp46, %76
  %divtmp48 = sdiv i32 %multmp45, %addtmp47
  store i32 %divtmp48, ptr @result, align 4
  %77 = load i32, ptr @result, align 4
  %78 = call ptr @tspp_int_to_string(i32 %77)
  call void @tspp_console_log(ptr %78)
  call void @tspp_free_string(ptr %78)
  %79 = load i32, ptr @a, align 4
  %80 = load i32, ptr @b, align 4
  %multmp49 = mul i32 %79, %80
  %81 = load i32, ptr @c, align 4
  %82 = load i32, ptr @d, align 4
  %multmp50 = mul i32 %81, %82
  %addtmp51 = add i32 %multmp49, %multmp50
  %83 = load i32, ptr @a, align 4
  %84 = load i32, ptr @c, align 4
  %addtmp52 = add i32 %83, %84
  %85 = load i32, ptr @b, align 4
  %86 = load i32, ptr @d, align 4
  %subtmp53 = sub i32 %85, %86
  %multmp54 = mul i32 %addtmp52, %subtmp53
  %subtmp55 = sub i32 %addtmp51, %multmp54
  %87 = load i32, ptr @a, align 4
  %88 = load i32, ptr @b, align 4
  %subtmp56 = sub i32 %87, %88
  %89 = load i32, ptr @c, align 4
  %90 = load i32, ptr @d, align 4
  %addtmp57 = add i32 %89, %90
  %addtmp58 = add i32 %subtmp56, %addtmp57
  %divtmp59 = sdiv i32 %subtmp55, %addtmp58
  store i32 %divtmp59, ptr @result, align 4
  %91 = load i32, ptr @result, align 4
  %92 = call ptr @tspp_int_to_string(i32 %91)
  call void @tspp_console_log(ptr %92)
  call void @tspp_free_string(ptr %92)
  %93 = load i32, ptr @a, align 4
  %94 = load i32, ptr @b, align 4
  %icmpogt60 = icmp sgt i32 %93, %94
  br i1 %icmpogt60, label %land.rhs61, label %land.end62

lor.rhs:                                          ; preds = %land.rhs21
  %95 = load i32, ptr @a, align 4
  %96 = load i32, ptr @c, align 4
  %addtmp24 = add i32 %95, %96
  %97 = load i32, ptr @b, align 4
  %98 = load i32, ptr @d, align 4
  %addtmp25 = add i32 %97, %98
  %icmpolt26 = icmp slt i32 %addtmp24, %addtmp25
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.rhs21
  %lor = phi i1 [ true, %land.rhs21 ], [ %icmpolt26, %lor.rhs ]
  br label %land.end22

land.rhs61:                                       ; preds = %land.end22
  %99 = load i32, ptr @c, align 4
  %100 = load i32, ptr @d, align 4
  %icmpogt63 = icmp sgt i32 %99, %100
  br label %land.end62

land.end62:                                       ; preds = %land.rhs61, %land.end22
  %land64 = phi i1 [ false, %land.end22 ], [ %icmpogt63, %land.rhs61 ]
  br i1 %land64, label %lor.end66, label %lor.rhs65

lor.rhs65:                                        ; preds = %land.end62
  %101 = load i32, ptr @a, align 4
  %102 = load i32, ptr @b, align 4
  %icmpolt67 = icmp slt i32 %101, %102
  br i1 %icmpolt67, label %land.rhs68, label %land.end69

lor.end66:                                        ; preds = %land.end69, %land.end62
  %lor72 = phi i1 [ true, %land.end62 ], [ %land71, %land.end69 ]
  store i1 %lor72, ptr @flag, align 1
  %103 = load i1, ptr @flag, align 1
  %104 = call ptr @tspp_bool_to_string(i1 %103)
  call void @tspp_console_log(ptr %104)
  call void @tspp_free_string(ptr %104)
  %105 = load i32, ptr @a, align 4
  %106 = load i32, ptr @b, align 4
  %icmpolt73 = icmp slt i32 %105, %106
  br i1 %icmpolt73, label %lor.end75, label %lor.rhs74

land.rhs68:                                       ; preds = %lor.rhs65
  %107 = load i32, ptr @c, align 4
  %108 = load i32, ptr @d, align 4
  %icmpolt70 = icmp slt i32 %107, %108
  br label %land.end69

land.end69:                                       ; preds = %land.rhs68, %lor.rhs65
  %land71 = phi i1 [ false, %lor.rhs65 ], [ %icmpolt70, %land.rhs68 ]
  br label %lor.end66

lor.rhs74:                                        ; preds = %lor.end66
  %109 = load i32, ptr @c, align 4
  %110 = load i32, ptr @d, align 4
  %icmpolt76 = icmp slt i32 %109, %110
  br label %lor.end75

lor.end75:                                        ; preds = %lor.rhs74, %lor.end66
  %lor77 = phi i1 [ true, %lor.end66 ], [ %icmpolt76, %lor.rhs74 ]
  %not = xor i1 %lor77, true
  store i1 %not, ptr @flag, align 1
  %111 = load i1, ptr @flag, align 1
  %112 = call ptr @tspp_bool_to_string(i1 %111)
  call void @tspp_console_log(ptr %112)
  call void @tspp_free_string(ptr %112)
  %113 = call ptr @tspp_string_concat(ptr @3, ptr @2)
  %114 = call ptr @tspp_string_concat(ptr %113, ptr @3)
  call void @tspp_console_log(ptr %114)
  ret i1 false
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
