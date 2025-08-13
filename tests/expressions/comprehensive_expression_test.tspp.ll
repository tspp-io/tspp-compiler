; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 15
@b = global i32 4
@c = global i32 0
@flag1 = global i1 false
@flag2 = global i1 true
@0 = private unnamed_addr constant [19 x i8] c"=== ARITHMETIC ===\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@2 = private unnamed_addr constant [19 x i8] c"=== COMPARISON ===\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@4 = private unnamed_addr constant [16 x i8] c"=== LOGICAL ===\00", align 1
@5 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@6 = private unnamed_addr constant [14 x i8] c"=== UNARY ===\00", align 1
@7 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@8 = private unnamed_addr constant [16 x i8] c"=== COMPLEX ===\00", align 1
@9 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

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
  store i32 %addtmp, ptr @c, align 4
  %4 = load i32, ptr @c, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = load i32, ptr @a, align 4
  %7 = load i32, ptr @b, align 4
  %subtmp = sub i32 %6, %7
  store i32 %subtmp, ptr @c, align 4
  %8 = load i32, ptr @c, align 4
  %9 = call ptr @tspp_int_to_string(i32 %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %9)
  %10 = load i32, ptr @a, align 4
  %11 = load i32, ptr @b, align 4
  %multmp = mul i32 %10, %11
  store i32 %multmp, ptr @c, align 4
  %12 = load i32, ptr @c, align 4
  %13 = call ptr @tspp_int_to_string(i32 %12)
  call void @tspp_console_log(ptr %13)
  call void @tspp_free_string(ptr %13)
  %14 = load i32, ptr @a, align 4
  %15 = load i32, ptr @b, align 4
  %divtmp = sdiv i32 %14, %15
  store i32 %divtmp, ptr @c, align 4
  %16 = load i32, ptr @c, align 4
  %17 = call ptr @tspp_int_to_string(i32 %16)
  call void @tspp_console_log(ptr %17)
  call void @tspp_free_string(ptr %17)
  %18 = load i32, ptr @a, align 4
  %19 = load i32, ptr @b, align 4
  %modtmp = srem i32 %18, %19
  store i32 %modtmp, ptr @c, align 4
  %20 = load i32, ptr @c, align 4
  %21 = call ptr @tspp_int_to_string(i32 %20)
  call void @tspp_console_log(ptr %21)
  call void @tspp_free_string(ptr %21)
  %22 = call ptr @tspp_string_concat(ptr @3, ptr @2)
  %23 = call ptr @tspp_string_concat(ptr %22, ptr @3)
  call void @tspp_console_log(ptr %23)
  %24 = load i32, ptr @a, align 4
  %25 = load i32, ptr @b, align 4
  %icmpolt = icmp slt i32 %24, %25
  store i1 %icmpolt, ptr @flag1, align 1
  %26 = load i1, ptr @flag1, align 1
  %27 = call ptr @tspp_bool_to_string(i1 %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %27)
  %28 = load i32, ptr @a, align 4
  %29 = load i32, ptr @b, align 4
  %icmpogt = icmp sgt i32 %28, %29
  store i1 %icmpogt, ptr @flag1, align 1
  %30 = load i1, ptr @flag1, align 1
  %31 = call ptr @tspp_bool_to_string(i1 %30)
  call void @tspp_console_log(ptr %31)
  call void @tspp_free_string(ptr %31)
  %32 = load i32, ptr @a, align 4
  %33 = load i32, ptr @b, align 4
  %icmpole = icmp sle i32 %32, %33
  store i1 %icmpole, ptr @flag1, align 1
  %34 = load i1, ptr @flag1, align 1
  %35 = call ptr @tspp_bool_to_string(i1 %34)
  call void @tspp_console_log(ptr %35)
  call void @tspp_free_string(ptr %35)
  %36 = load i32, ptr @a, align 4
  %37 = load i32, ptr @b, align 4
  %icmpoge = icmp sge i32 %36, %37
  store i1 %icmpoge, ptr @flag1, align 1
  %38 = load i1, ptr @flag1, align 1
  %39 = call ptr @tspp_bool_to_string(i1 %38)
  call void @tspp_console_log(ptr %39)
  call void @tspp_free_string(ptr %39)
  %40 = load i32, ptr @a, align 4
  %41 = load i32, ptr @b, align 4
  %icmpeq = icmp eq i32 %40, %41
  store i1 %icmpeq, ptr @flag1, align 1
  %42 = load i1, ptr @flag1, align 1
  %43 = call ptr @tspp_bool_to_string(i1 %42)
  call void @tspp_console_log(ptr %43)
  call void @tspp_free_string(ptr %43)
  %44 = load i32, ptr @a, align 4
  %45 = load i32, ptr @b, align 4
  %icmpne = icmp ne i32 %44, %45
  store i1 %icmpne, ptr @flag1, align 1
  %46 = load i1, ptr @flag1, align 1
  %47 = call ptr @tspp_bool_to_string(i1 %46)
  call void @tspp_console_log(ptr %47)
  call void @tspp_free_string(ptr %47)
  %48 = call ptr @tspp_string_concat(ptr @5, ptr @4)
  %49 = call ptr @tspp_string_concat(ptr %48, ptr @5)
  call void @tspp_console_log(ptr %49)
  %50 = load i1, ptr @flag2, align 1
  br i1 %50, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %51 = load i32, ptr @a, align 4
  %52 = load i32, ptr @b, align 4
  %icmpogt1 = icmp sgt i32 %51, %52
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %icmpogt1, %land.rhs ]
  store i1 %land, ptr @flag1, align 1
  %53 = load i1, ptr @flag1, align 1
  %54 = call ptr @tspp_bool_to_string(i1 %53)
  call void @tspp_console_log(ptr %54)
  call void @tspp_free_string(ptr %54)
  %55 = load i1, ptr @flag1, align 1
  br i1 %55, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %land.end
  %56 = load i32, ptr @a, align 4
  %57 = load i32, ptr @b, align 4
  %icmpolt2 = icmp slt i32 %56, %57
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.end
  %lor = phi i1 [ true, %land.end ], [ %icmpolt2, %lor.rhs ]
  store i1 %lor, ptr @flag1, align 1
  %58 = load i1, ptr @flag1, align 1
  %59 = call ptr @tspp_bool_to_string(i1 %58)
  call void @tspp_console_log(ptr %59)
  call void @tspp_free_string(ptr %59)
  %60 = call ptr @tspp_string_concat(ptr @7, ptr @6)
  %61 = call ptr @tspp_string_concat(ptr %60, ptr @7)
  call void @tspp_console_log(ptr %61)
  %62 = load i32, ptr @a, align 4
  %neg = sub i32 0, %62
  store i32 %neg, ptr @c, align 4
  %63 = load i32, ptr @c, align 4
  %64 = call ptr @tspp_int_to_string(i32 %63)
  call void @tspp_console_log(ptr %64)
  call void @tspp_free_string(ptr %64)
  %65 = load i1, ptr @flag2, align 1
  %not = xor i1 %65, true
  store i1 %not, ptr @flag1, align 1
  %66 = load i1, ptr @flag1, align 1
  %67 = call ptr @tspp_bool_to_string(i1 %66)
  call void @tspp_console_log(ptr %67)
  call void @tspp_free_string(ptr %67)
  %68 = call ptr @tspp_string_concat(ptr @9, ptr @8)
  %69 = call ptr @tspp_string_concat(ptr %68, ptr @9)
  call void @tspp_console_log(ptr %69)
  %70 = load i32, ptr @a, align 4
  %71 = load i32, ptr @b, align 4
  %multmp3 = mul i32 %71, 2
  %addtmp4 = add i32 %70, %multmp3
  store i32 %addtmp4, ptr @c, align 4
  %72 = load i32, ptr @c, align 4
  %73 = call ptr @tspp_int_to_string(i32 %72)
  call void @tspp_console_log(ptr %73)
  call void @tspp_free_string(ptr %73)
  %74 = load i32, ptr @a, align 4
  %75 = load i32, ptr @b, align 4
  %addtmp5 = add i32 %74, %75
  %multmp6 = mul i32 %addtmp5, 2
  store i32 %multmp6, ptr @c, align 4
  %76 = load i32, ptr @c, align 4
  %77 = call ptr @tspp_int_to_string(i32 %76)
  call void @tspp_console_log(ptr %77)
  call void @tspp_free_string(ptr %77)
  %78 = load i32, ptr @a, align 4
  %icmpogt7 = icmp sgt i32 %78, 10
  br i1 %icmpogt7, label %land.rhs8, label %land.end9

land.rhs8:                                        ; preds = %lor.end
  %79 = load i32, ptr @b, align 4
  %icmpolt10 = icmp slt i32 %79, 10
  br label %land.end9

land.end9:                                        ; preds = %land.rhs8, %lor.end
  %land11 = phi i1 [ false, %lor.end ], [ %icmpolt10, %land.rhs8 ]
  br i1 %land11, label %lor.end13, label %lor.rhs12

lor.rhs12:                                        ; preds = %land.end9
  %80 = load i32, ptr @a, align 4
  %icmpeq14 = icmp eq i32 %80, 15
  br label %lor.end13

lor.end13:                                        ; preds = %lor.rhs12, %land.end9
  %lor15 = phi i1 [ true, %land.end9 ], [ %icmpeq14, %lor.rhs12 ]
  store i1 %lor15, ptr @flag1, align 1
  %81 = load i1, ptr @flag1, align 1
  %82 = call ptr @tspp_bool_to_string(i1 %81)
  call void @tspp_console_log(ptr %82)
  call void @tspp_free_string(ptr %82)
  %83 = load i32, ptr @b, align 4
  %addtmp16 = add i32 %83, 1
  store i32 %addtmp16, ptr @a, align 4
  %84 = load i32, ptr @a, align 4
  %85 = call ptr @tspp_int_to_string(i32 %84)
  call void @tspp_console_log(ptr %85)
  call void @tspp_free_string(ptr %85)
  ret i1 false
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
