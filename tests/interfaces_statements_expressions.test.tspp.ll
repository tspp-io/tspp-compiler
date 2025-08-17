; ModuleID = 'tspp_module'
source_filename = "tspp_module"

%Point = type { float, float }
%SBox = type { ptr }

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [2 x i8] c"(\00", align 1
@1 = private unnamed_addr constant [3 x i8] c", \00", align 1
@2 = private unnamed_addr constant [2 x i8] c")\00", align 1
@3 = private unnamed_addr constant [6 x i8] c"Point\00", align 1
@4 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@5 = private unnamed_addr constant [2 x i8] c"!\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@7 = private unnamed_addr constant [7 x i8] c"dist: \00", align 1
@8 = private unnamed_addr constant [8 x i8] c"ident: \00", align 1
@9 = private unnamed_addr constant [11 x i8] c"flexible: \00", align 1
@10 = private unnamed_addr constant [5 x i8] c"p1: \00", align 1
@11 = private unnamed_addr constant [11 x i8] c"p1 moved: \00", align 1
@12 = private unnamed_addr constant [21 x i8] c"stringifyPoint(p1): \00", align 1
@13 = private unnamed_addr constant [3 x i8] c"hi\00", align 1
@14 = private unnamed_addr constant [3 x i8] c"hi\00", align 1
@15 = private unnamed_addr constant [19 x i8] c"useBoxString(sb): \00", align 1
@16 = private unnamed_addr constant [12 x i8] c"isEven(6): \00", align 1
@17 = private unnamed_addr constant [12 x i8] c"isEven(6): \00", align 1
@18 = private unnamed_addr constant [11 x i8] c"sumTo(5): \00", align 1
@19 = private unnamed_addr constant [16 x i8] c"forVariants(): \00", align 1
@20 = private unnamed_addr constant [19 x i8] c"precedenceDemo(): \00", align 1
@21 = private unnamed_addr constant [25 x i8] c"memberAndIndexAccess(): \00", align 1
@22 = private unnamed_addr constant [4 x i8] c"x: \00", align 1
@23 = private unnamed_addr constant [4 x i8] c"y: \00", align 1
@24 = private unnamed_addr constant [10 x i8] c"logical: \00", align 1
@25 = private unnamed_addr constant [7 x i8] c"1<<3: \00", align 1
@26 = private unnamed_addr constant [7 x i8] c"8>>1: \00", align 1
@27 = private unnamed_addr constant [5 x i8] c"~0: \00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define void @__ctor_Point(ptr %this, float %x, float %y) {
entry:
  %y3 = alloca float, align 4
  %x2 = alloca float, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store float %x, ptr %x2, align 4
  store float %y, ptr %y3, align 4
  %0 = load float, ptr %x2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Point, ptr %1, i32 0, i32 0
  store float %0, ptr %field.ptr, align 4
  %2 = load float, ptr %y3, align 4
  %3 = load ptr, ptr %this1, align 8
  %field.ptr4 = getelementptr %Point, ptr %3, i32 0, i32 1
  store float %2, ptr %field.ptr4, align 4
  ret void
}

define ptr @Point.toString(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Point, ptr %0, i32 0, i32 0
  %1 = load float, ptr %field.ptr, align 4
  %2 = call ptr @tspp_float_to_string(float %1)
  %3 = call ptr @tspp_string_concat(ptr @0, ptr %2)
  %4 = call ptr @tspp_string_concat(ptr %3, ptr @1)
  %5 = load ptr, ptr %this1, align 8
  %field.ptr2 = getelementptr %Point, ptr %5, i32 0, i32 1
  %6 = load float, ptr %field.ptr2, align 4
  %7 = call ptr @tspp_float_to_string(float %6)
  %8 = call ptr @tspp_string_concat(ptr %4, ptr %7)
  %9 = call ptr @tspp_string_concat(ptr %8, ptr @2)
  ret ptr %9
}

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_string_concat(ptr, ptr)

define void @Point.move(ptr %this, float %dx, float %dy) {
entry:
  %dy3 = alloca float, align 4
  %dx2 = alloca float, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store float %dx, ptr %dx2, align 4
  store float %dy, ptr %dy3, align 4
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Point, ptr %0, i32 0, i32 0
  %1 = load float, ptr %field.ptr, align 4
  %2 = load float, ptr %dx2, align 4
  %faddtmp = fadd float %1, %2
  %3 = load ptr, ptr %this1, align 8
  %field.ptr4 = getelementptr %Point, ptr %3, i32 0, i32 0
  store float %faddtmp, ptr %field.ptr4, align 4
  %4 = load ptr, ptr %this1, align 8
  %field.ptr5 = getelementptr %Point, ptr %4, i32 0, i32 1
  %5 = load float, ptr %field.ptr5, align 4
  %6 = load float, ptr %dy3, align 4
  %faddtmp6 = fadd float %5, %6
  %7 = load ptr, ptr %this1, align 8
  %field.ptr7 = getelementptr %Point, ptr %7, i32 0, i32 1
  store float %faddtmp6, ptr %field.ptr7, align 4
  ret void
}

define void @__ctor_SBox(ptr %this, ptr %v) {
entry:
  %v2 = alloca ptr, align 8
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store ptr %v, ptr %v2, align 8
  %0 = load ptr, ptr %v2, align 8
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %SBox, ptr %1, i32 0, i32 0
  store ptr %0, ptr %field.ptr, align 8
  ret void
}

define ptr @SBox.get(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %SBox, ptr %0, i32 0, i32 0
  %1 = load ptr, ptr %field.ptr, align 8
  ret ptr %1
}

define void @SBox.set(ptr %this, ptr %v) {
entry:
  %v2 = alloca ptr, align 8
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store ptr %v, ptr %v2, align 8
  %0 = load ptr, ptr %v2, align 8
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %SBox, ptr %1, i32 0, i32 0
  store ptr %0, ptr %field.ptr, align 8
  ret void
}

define i1 @isEven(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  %modtmp = srem i32 %0, 2
  %icmpeq = icmp eq i32 %modtmp, 0
  ret i1 %icmpeq
}

define i32 @sumTo(i32 %n) {
entry:
  %i = alloca i32, align 4
  %s = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store i32 0, ptr %s, align 4
  store i32 1, ptr %i, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %n1, align 4
  %icmpole = icmp sle i32 %0, %1
  br i1 %icmpole, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %2 = load i32, ptr %s, align 4
  %3 = load i32, ptr %i, align 4
  %addtmp = add i32 %2, %3
  store i32 %addtmp, ptr %s, align 4
  %4 = load i32, ptr %i, align 4
  %addtmp2 = add i32 %4, 1
  store i32 %addtmp2, ptr %i, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %5 = load i32, ptr %s, align 4
  ret i32 %5
}

define i32 @forVariants() {
entry:
  %k = alloca i32, align 4
  %j = alloca i32, align 4
  %i = alloca i32, align 4
  %total = alloca i32, align 4
  store i32 0, ptr %total, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %icmpolt = icmp slt i32 %0, 3
  br i1 %icmpolt, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %total, align 4
  %2 = load i32, ptr %i, align 4
  %addtmp = add i32 %1, %2
  store i32 %addtmp, ptr %total, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr %i, align 4
  %addtmp1 = add i32 %3, 1
  store i32 %addtmp1, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, ptr %j, align 4
  store i32 0, ptr %j, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc4, %for.end
  %4 = load i32, ptr %j, align 4
  %icmpolt6 = icmp slt i32 %4, 2
  br i1 %icmpolt6, label %for.body3, label %for.end5

for.body3:                                        ; preds = %for.cond2
  %5 = load i32, ptr %total, align 4
  %addtmp7 = add i32 %5, 10
  store i32 %addtmp7, ptr %total, align 4
  br label %for.inc4

for.inc4:                                         ; preds = %for.body3
  %6 = load i32, ptr %j, align 4
  %addtmp8 = add i32 %6, 1
  store i32 %addtmp8, ptr %j, align 4
  br label %for.cond2

for.end5:                                         ; preds = %for.cond2
  store i32 0, ptr %k, align 4
  br label %for.cond9

for.cond9:                                        ; preds = %for.inc11, %for.end5
  %7 = load i32, ptr %k, align 4
  %icmpolt13 = icmp slt i32 %7, 2
  br i1 %icmpolt13, label %for.body10, label %for.end12

for.body10:                                       ; preds = %for.cond9
  %8 = load i32, ptr %total, align 4
  %addtmp14 = add i32 %8, 100
  store i32 %addtmp14, ptr %total, align 4
  br label %for.inc11

for.inc11:                                        ; preds = %for.body10
  %9 = load i32, ptr %k, align 4
  %addtmp15 = add i32 %9, 1
  store i32 %addtmp15, ptr %k, align 4
  br label %for.cond9

for.end12:                                        ; preds = %for.cond9
  %10 = load i32, ptr %total, align 4
  ret i32 %10
}

define i32 @precedenceDemo() {
entry:
  %sum = alloca i32, align 4
  %logic = alloca i1, align 1
  %rel = alloca i1, align 1
  %eq = alloca i1, align 1
  %not0 = alloca i32, align 4
  %bxor = alloca i32, align 4
  %bor = alloca i32, align 4
  %band = alloca i32, align 4
  %shiftMix = alloca i32, align 4
  %addMul = alloca i32, align 4
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 5, ptr %a, align 4
  store i32 3, ptr %b, align 4
  store i32 14, ptr %addMul, align 4
  store i32 16, ptr %shiftMix, align 4
  %0 = load i32, ptr %a, align 4
  %1 = load i32, ptr %b, align 4
  %andtmp = and i32 %0, %1
  store i32 %andtmp, ptr %band, align 4
  %2 = load i32, ptr %a, align 4
  %3 = load i32, ptr %b, align 4
  %ortmp = or i32 %2, %3
  store i32 %ortmp, ptr %bor, align 4
  %4 = load i32, ptr %a, align 4
  %5 = load i32, ptr %b, align 4
  %xortmp = xor i32 %4, %5
  store i32 %xortmp, ptr %bxor, align 4
  store i32 -1, ptr %not0, align 4
  %6 = load i32, ptr %addMul, align 4
  %icmpeq = icmp eq i32 %6, 14
  store i1 %icmpeq, ptr %eq, align 1
  %7 = load i32, ptr %shiftMix, align 4
  %icmpoge = icmp sge i32 %7, 16
  store i1 %icmpoge, ptr %rel, align 1
  %8 = load i1, ptr %eq, align 1
  br i1 %8, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %9 = load i1, ptr %rel, align 1
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %9, %land.rhs ]
  br i1 %land, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %land.end
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.end
  %lor = phi i1 [ true, %land.end ], [ false, %lor.rhs ]
  store i1 %lor, ptr %logic, align 1
  %10 = load i32, ptr %addMul, align 4
  %11 = load i32, ptr %shiftMix, align 4
  %addtmp = add i32 %10, %11
  %12 = load i32, ptr %band, align 4
  %addtmp1 = add i32 %addtmp, %12
  %13 = load i32, ptr %bor, align 4
  %addtmp2 = add i32 %addtmp1, %13
  %14 = load i32, ptr %bxor, align 4
  %addtmp3 = add i32 %addtmp2, %14
  %15 = load i32, ptr %not0, align 4
  %addtmp4 = add i32 %addtmp3, %15
  %16 = load i1, ptr %logic, align 1
  %17 = zext i1 %16 to i32
  %addtmp5 = add i32 %addtmp4, %17
  store i32 %addtmp5, ptr %sum, align 4
  %18 = load i32, ptr %sum, align 4
  ret i32 %18
}

define i32 @memberAndIndexAccess() {
entry:
  %chk = alloca i32, align 4
  %arr1 = alloca ptr, align 8
  %p = alloca ptr, align 8
  %obj = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Point, ptr null, i32 1) to i64))
  call void @__ctor_Point(ptr %obj, float 1.000000e+00, float 2.000000e+00)
  store ptr %obj, ptr %p, align 8
  %0 = load ptr, ptr %p, align 8
  call void @Point.move(ptr %0, float 1.000000e+00, float 3.000000e+00)
  %arr = call ptr @GC_malloc(i64 12)
  %1 = getelementptr i32, ptr %arr, i32 0
  store i32 10, ptr %1, align 4
  %2 = getelementptr i32, ptr %arr, i32 1
  store i32 20, ptr %2, align 4
  %3 = getelementptr i32, ptr %arr, i32 2
  store i32 30, ptr %3, align 4
  store ptr %arr, ptr %arr1, align 8
  %4 = load ptr, ptr %arr1, align 8
  %elem.ptr = getelementptr i32, ptr %4, i32 1
  %5 = load i32, ptr %elem.ptr, align 4
  %addtmp = add i32 %5, 79
  %6 = load ptr, ptr %arr1, align 8
  %elem.ptr2 = getelementptr i32, ptr %6, i32 1
  store i32 %addtmp, ptr %elem.ptr2, align 4
  %7 = load ptr, ptr %arr1, align 8
  %elem.ptr3 = getelementptr i32, ptr %7, i32 0
  %8 = load i32, ptr %elem.ptr3, align 4
  %9 = load ptr, ptr %arr1, align 8
  %elem.ptr4 = getelementptr i32, ptr %9, i32 1
  %10 = load i32, ptr %elem.ptr4, align 4
  %addtmp5 = add i32 %8, %10
  %11 = load ptr, ptr %arr1, align 8
  %elem.ptr6 = getelementptr i32, ptr %11, i32 2
  %12 = load i32, ptr %elem.ptr6, align 4
  %addtmp7 = add i32 %addtmp5, %12
  %13 = load ptr, ptr %p, align 8
  %field.ptr = getelementptr %Point, ptr %13, i32 0, i32 0
  %14 = load float, ptr %field.ptr, align 4
  %sitofp = sitofp i32 %addtmp7 to float
  %faddtmp = fadd float %sitofp, %14
  %15 = load ptr, ptr %p, align 8
  %field.ptr8 = getelementptr %Point, ptr %15, i32 0, i32 1
  %16 = load float, ptr %field.ptr8, align 4
  %faddtmp9 = fadd float %faddtmp, %16
  %17 = fptosi float %faddtmp9 to i32
  store i32 %17, ptr %chk, align 4
  %18 = load i32, ptr %chk, align 4
  ret i32 %18
}

declare ptr @GC_malloc(i64)

define ptr @stringifyPoint(ptr %pt) {
entry:
  %pt1 = alloca ptr, align 8
  store ptr %pt, ptr %pt1, align 8
  %0 = load ptr, ptr %pt1, align 8
  %1 = call ptr @tspp_string_concat(ptr @3, ptr @4)
  ret ptr %1
}

define ptr @useBoxString(ptr %bx) {
entry:
  %bx1 = alloca ptr, align 8
  store ptr %bx, ptr %bx1, align 8
  %0 = load ptr, ptr %bx1, align 8
  %1 = load ptr, ptr %bx1, align 8
  %2 = call ptr @SBox.get(ptr %1)
  %3 = call ptr @tspp_ptr_to_string(ptr %2)
  %4 = call ptr @tspp_string_concat(ptr %3, ptr @5)
  call void @SBox.set(ptr %0, ptr %4)
  %5 = load ptr, ptr %bx1, align 8
  %6 = call ptr @SBox.get(ptr %5)
  ret ptr %6
}

declare ptr @tspp_ptr_to_string(ptr)

define i32 @main() {
entry:
  %L = alloca i1, align 1
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  %n = alloca i32, align 4
  %sb = alloca ptr, align 8
  %p1 = alloca ptr, align 8
  %flexible = alloca ptr, align 8
  %ident = alloca i32, align 4
  %dist = alloca float, align 4
  store float 3.500000e+00, ptr %dist, align 4
  store i32 42, ptr %ident, align 4
  store ptr @6, ptr %flexible, align 8
  %0 = load float, ptr %dist, align 4
  %1 = call ptr @tspp_float_to_string(float %0)
  %2 = call ptr @tspp_string_concat(ptr @7, ptr %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %1)
  %3 = load i32, ptr %ident, align 4
  %4 = call ptr @tspp_int_to_string(i32 %3)
  %5 = call ptr @tspp_string_concat(ptr @8, ptr %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %4)
  %6 = load ptr, ptr %flexible, align 8
  %7 = call ptr @tspp_string_concat(ptr @9, ptr %6)
  call void @tspp_console_log(ptr %7)
  %obj = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Point, ptr null, i32 1) to i64))
  call void @__ctor_Point(ptr %obj, float 1.500000e+00, float 2.500000e+00)
  store ptr %obj, ptr %p1, align 8
  %8 = load ptr, ptr %p1, align 8
  %9 = call ptr @Point.toString(ptr %8)
  %10 = call ptr @tspp_string_concat(ptr @10, ptr %9)
  call void @tspp_console_log(ptr %10)
  %11 = load ptr, ptr %p1, align 8
  call void @Point.move(ptr %11, float 5.000000e-01, float 5.000000e-01)
  %12 = load ptr, ptr %p1, align 8
  %13 = call ptr @Point.toString(ptr %12)
  %14 = call ptr @tspp_string_concat(ptr @11, ptr %13)
  call void @tspp_console_log(ptr %14)
  %15 = load ptr, ptr %p1, align 8
  %16 = call ptr @stringifyPoint(ptr %15)
  %17 = call ptr @tspp_string_concat(ptr @12, ptr %16)
  call void @tspp_console_log(ptr %17)
  %18 = load ptr, ptr %sb, align 8
  %19 = call ptr @useBoxString(ptr %18)
  %20 = call ptr @tspp_string_concat(ptr @15, ptr %19)
  call void @tspp_console_log(ptr %20)
  store i32 6, ptr %n, align 4
  %21 = load i32, ptr %n, align 4
  %22 = call i1 @isEven(i32 %21)
  br i1 %22, label %then, label %else

then:                                             ; preds = %entry
  %23 = call ptr @tspp_bool_to_string(i1 true)
  %24 = call ptr @tspp_string_concat(ptr @16, ptr %23)
  call void @tspp_console_log(ptr %24)
  call void @tspp_free_string(ptr %23)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %25 = call i32 @sumTo(i32 5)
  %26 = call ptr @tspp_int_to_string(i32 %25)
  %27 = call ptr @tspp_string_concat(ptr @18, ptr %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %26)
  %28 = call i32 @forVariants()
  %29 = call ptr @tspp_int_to_string(i32 %28)
  %30 = call ptr @tspp_string_concat(ptr @19, ptr %29)
  call void @tspp_console_log(ptr %30)
  call void @tspp_free_string(ptr %29)
  %31 = call i32 @precedenceDemo()
  %32 = call ptr @tspp_int_to_string(i32 %31)
  %33 = call ptr @tspp_string_concat(ptr @20, ptr %32)
  call void @tspp_console_log(ptr %33)
  call void @tspp_free_string(ptr %32)
  %34 = call i32 @memberAndIndexAccess()
  %35 = call ptr @tspp_int_to_string(i32 %34)
  %36 = call ptr @tspp_string_concat(ptr @21, ptr %35)
  call void @tspp_console_log(ptr %36)
  call void @tspp_free_string(ptr %35)
  store i32 0, ptr %x, align 4
  store i32 0, ptr %y, align 4
  store i32 5, ptr %y, align 4
  store i32 5, ptr %x, align 4
  %37 = load i32, ptr %x, align 4
  %38 = call ptr @tspp_int_to_string(i32 %37)
  %39 = call ptr @tspp_string_concat(ptr @22, ptr %38)
  call void @tspp_console_log(ptr %39)
  call void @tspp_free_string(ptr %38)
  %40 = load i32, ptr %y, align 4
  %41 = call ptr @tspp_int_to_string(i32 %40)
  %42 = call ptr @tspp_string_concat(ptr @23, ptr %41)
  call void @tspp_console_log(ptr %42)
  call void @tspp_free_string(ptr %41)
  br i1 false, label %land.rhs, label %land.end

else:                                             ; preds = %entry
  %43 = call ptr @tspp_bool_to_string(i1 false)
  %44 = call ptr @tspp_string_concat(ptr @17, ptr %43)
  call void @tspp_console_log(ptr %44)
  call void @tspp_free_string(ptr %43)
  br label %ifcont

land.rhs:                                         ; preds = %ifcont
  br label %land.end

land.end:                                         ; preds = %land.rhs, %ifcont
  %land = phi i1 [ false, %ifcont ], [ poison, %land.rhs ]
  br i1 %land, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %land.end
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.end
  %lor = phi i1 [ true, %land.end ], [ true, %lor.rhs ]
  store i1 %lor, ptr %L, align 1
  %45 = load i1, ptr %L, align 1
  %46 = call ptr @tspp_bool_to_string(i1 %45)
  %47 = call ptr @tspp_string_concat(ptr @24, ptr %46)
  call void @tspp_console_log(ptr %47)
  call void @tspp_free_string(ptr %46)
  %48 = call ptr @tspp_int_to_string(i32 8)
  %49 = call ptr @tspp_string_concat(ptr @25, ptr %48)
  call void @tspp_console_log(ptr %49)
  call void @tspp_free_string(ptr %48)
  %50 = call ptr @tspp_int_to_string(i32 4)
  %51 = call ptr @tspp_string_concat(ptr @26, ptr %50)
  call void @tspp_console_log(ptr %51)
  call void @tspp_free_string(ptr %50)
  %52 = call ptr @tspp_int_to_string(i32 -1)
  %53 = call ptr @tspp_string_concat(ptr @27, ptr %52)
  call void @tspp_console_log(ptr %53)
  call void @tspp_free_string(ptr %52)
  ret i32 0
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_bool_to_string(i1)
