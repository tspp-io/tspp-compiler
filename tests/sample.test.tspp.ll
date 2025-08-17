; ModuleID = 'tspp_module'
source_filename = "tspp_module"
target triple = "x86_64-pc-linux-gnu"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 42
@b = global float 0x40091EB860000000
@c = global i1 true
@0 = private constant [6 x i8] c"Hello\00"
@1 = private constant [6 x i8] c"Hello\00"
@d = global ptr @1
@e = global i32 10
@f = global float 0x40239EB860000000
@g = internal global i1 false
@2 = private constant [7 x i8] c"either\00"
@3 = private constant [7 x i8] c"either\00"
@h = global i32 @3
@i = global ptr null
@j = global ptr null
@k = global ptr null
@l = global ptr null
@m = global ptr null
@n = global i32 0
@o = global i32 0
@p = global i32 0
@q = global i32 100
@r = global ptr null
@s = global i32 5
@t = global i1 false
@4 = private unnamed_addr constant [8 x i8] c"Hello, \00", align 1
@5 = private unnamed_addr constant [2 x i8] c"(\00", align 1
@6 = private unnamed_addr constant [3 x i8] c", \00", align 1
@7 = private unnamed_addr constant [2 x i8] c")\00", align 1
@8 = private unnamed_addr constant [11 x i8] c"add(2,3): \00", align 1
@9 = private unnamed_addr constant [10 x i8] c"greet(): \00", align 1
@10 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@11 = private unnamed_addr constant [17 x i8] c"greet('Alice'): \00", align 1
@12 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@13 = private unnamed_addr constant [21 x i8] c"multiply(2.5, 4.0): \00", align 1
@14 = private unnamed_addr constant [19 x i8] c"formatPoint(1.1): \00", align 1
@15 = private unnamed_addr constant [2 x i8] c"P\00", align 1
@16 = private unnamed_addr constant [28 x i8] c"formatPoint(2.2, 3.3,'Q'): \00", align 1
@17 = private unnamed_addr constant [2 x i8] c"Q\00", align 1
@18 = private unnamed_addr constant [12 x i8] c"square(5): \00", align 1
@19 = private unnamed_addr constant [15 x i8] c"increment(7): \00", align 1
@20 = private unnamed_addr constant [4 x i8] c"a: \00", align 1
@21 = private unnamed_addr constant [4 x i8] c"b: \00", align 1
@22 = private unnamed_addr constant [4 x i8] c"c: \00", align 1
@23 = private unnamed_addr constant [4 x i8] c"d: \00", align 1
@24 = private unnamed_addr constant [4 x i8] c"e: \00", align 1
@25 = private unnamed_addr constant [4 x i8] c"f: \00", align 1
@26 = private unnamed_addr constant [4 x i8] c"g: \00", align 1
@27 = private unnamed_addr constant [4 x i8] c"h: \00", align 1
@28 = private unnamed_addr constant [4 x i8] c"i: \00", align 1
@29 = private unnamed_addr constant [4 x i8] c"j: \00", align 1
@30 = private unnamed_addr constant [4 x i8] c"k: \00", align 1
@31 = private unnamed_addr constant [4 x i8] c"l: \00", align 1
@32 = private unnamed_addr constant [4 x i8] c"m: \00", align 1
@33 = private unnamed_addr constant [4 x i8] c"n: \00", align 1
@34 = private unnamed_addr constant [4 x i8] c"o: \00", align 1
@35 = private unnamed_addr constant [4 x i8] c"p: \00", align 1
@36 = private unnamed_addr constant [4 x i8] c"q: \00", align 1
@37 = private unnamed_addr constant [4 x i8] c"r: \00", align 1
@38 = private unnamed_addr constant [4 x i8] c"s: \00", align 1
@39 = private unnamed_addr constant [4 x i8] c"t: \00", align 1
@40 = private unnamed_addr constant [3 x i8] c"{ \00", align 1
@41 = private unnamed_addr constant [5 x i8] c"name\00", align 1
@42 = private unnamed_addr constant [3 x i8] c": \00", align 1
@43 = private unnamed_addr constant [5 x i8] c"John\00", align 1
@44 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@45 = private unnamed_addr constant [3 x i8] c", \00", align 1
@46 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@47 = private unnamed_addr constant [3 x i8] c": \00", align 1
@48 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@49 = private unnamed_addr constant [3 x i8] c"{ \00", align 1
@50 = private unnamed_addr constant [5 x i8] c"unit\00", align 1
@51 = private unnamed_addr constant [3 x i8] c": \00", align 1
@52 = private unnamed_addr constant [2 x i8] c"m\00", align 1
@53 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@54 = private unnamed_addr constant [3 x i8] c", \00", align 1
@55 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@56 = private unnamed_addr constant [3 x i8] c": \00", align 1
@57 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@58 = private unnamed_addr constant [3 x i8] c"{ \00", align 1
@59 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@60 = private unnamed_addr constant [3 x i8] c": \00", align 1
@61 = private unnamed_addr constant [3 x i8] c", \00", align 1
@62 = private unnamed_addr constant [2 x i8] c"y\00", align 1
@63 = private unnamed_addr constant [3 x i8] c": \00", align 1
@64 = private unnamed_addr constant [3 x i8] c" }\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  %0 = call ptr @tspp_string_concat(ptr @40, ptr @41)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @42)
  %2 = call ptr @tspp_string_concat(ptr %1, ptr @44)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @43)
  %4 = call ptr @tspp_string_concat(ptr %3, ptr @44)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @45)
  %6 = call ptr @tspp_string_concat(ptr %5, ptr @46)
  %7 = call ptr @tspp_string_concat(ptr %6, ptr @47)
  %8 = call ptr @tspp_int_to_string(i32 1)
  %9 = call ptr @tspp_string_concat(ptr %7, ptr %8)
  %10 = call ptr @tspp_string_concat(ptr %9, ptr @48)
  store ptr %10, ptr @i, align 8
  %11 = call ptr @tspp_string_concat(ptr @49, ptr @50)
  %12 = call ptr @tspp_string_concat(ptr %11, ptr @51)
  %13 = call ptr @tspp_string_concat(ptr %12, ptr @53)
  %14 = call ptr @tspp_string_concat(ptr %13, ptr @52)
  %15 = call ptr @tspp_string_concat(ptr %14, ptr @53)
  %16 = call ptr @tspp_string_concat(ptr %15, ptr @54)
  %17 = call ptr @tspp_string_concat(ptr %16, ptr @55)
  %18 = call ptr @tspp_string_concat(ptr %17, ptr @56)
  %19 = call ptr @tspp_float_to_string(float 5.500000e+00)
  %20 = call ptr @tspp_string_concat(ptr %18, ptr %19)
  %21 = call ptr @tspp_string_concat(ptr %20, ptr @57)
  store ptr %21, ptr @j, align 8
  %22 = call ptr @tspp_string_concat(ptr @58, ptr @59)
  %23 = call ptr @tspp_string_concat(ptr %22, ptr @60)
  %24 = call ptr @tspp_float_to_string(float 0x3FF19999A0000000)
  %25 = call ptr @tspp_string_concat(ptr %23, ptr %24)
  %26 = call ptr @tspp_string_concat(ptr %25, ptr @61)
  %27 = call ptr @tspp_string_concat(ptr %26, ptr @62)
  %28 = call ptr @tspp_string_concat(ptr %27, ptr @63)
  %29 = call ptr @tspp_float_to_string(float 0x40019999A0000000)
  %30 = call ptr @tspp_string_concat(ptr %28, ptr %29)
  %31 = call ptr @tspp_string_concat(ptr %30, ptr @64)
  store ptr %31, ptr @r, align 8
  ret void
}

define i32 @add(i32 %x, i32 %y) {
entry:
  %y2 = alloca i32, align 4
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i32 %y, ptr %y2, align 4
  %0 = load i32, ptr %x1, align 4
  %1 = load i32, ptr %y2, align 4
  %addtmp = add i32 %0, %1
  ret i32 %addtmp
}

define ptr @greet(ptr %name) {
entry:
  %name1 = alloca ptr, align 8
  store ptr %name, ptr %name1, align 8
  %0 = load ptr, ptr %name1, align 8
  %1 = call ptr @tspp_string_concat(ptr @4, ptr %0)
  ret ptr %1
}

declare ptr @tspp_string_concat(ptr, ptr)

define float @multiply(float %a, float %b) {
entry:
  %b2 = alloca float, align 4
  %a1 = alloca float, align 4
  store float %a, ptr %a1, align 4
  store float %b, ptr %b2, align 4
  %0 = load float, ptr %a1, align 4
  %1 = load float, ptr %b2, align 4
  %fmultmp = fmul float %0, %1
  ret float %fmultmp
}

define ptr @formatPoint(float %x, float %y, ptr %label) {
entry:
  %label3 = alloca ptr, align 8
  %y2 = alloca float, align 4
  %x1 = alloca float, align 4
  store float %x, ptr %x1, align 4
  store float %y, ptr %y2, align 4
  store ptr %label, ptr %label3, align 8
  %0 = load ptr, ptr %label3, align 8
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @5)
  %2 = load float, ptr %x1, align 4
  %3 = call ptr @tspp_float_to_string(float %2)
  %4 = call ptr @tspp_string_concat(ptr %1, ptr %3)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @6)
  %6 = load float, ptr %y2, align 4
  %7 = call ptr @tspp_float_to_string(float %6)
  %8 = call ptr @tspp_string_concat(ptr %5, ptr %7)
  %9 = call ptr @tspp_string_concat(ptr %8, ptr @7)
  ret ptr %9
}

declare ptr @tspp_float_to_string(float)

define i32 @square(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  %1 = load i32, ptr %n1, align 4
  %multmp = mul i32 %0, %1
  ret i32 %multmp
}

define i32 @increment(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %0 = load i32, ptr %x1, align 4
  %addtmp = add i32 %0, 1
  ret i32 %addtmp
}

define i32 @testFunctions() {
entry:
  %0 = call i32 @add(i32 2, i32 3)
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call ptr @tspp_string_concat(ptr @8, ptr %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %1)
  %3 = call ptr @greet(ptr @10)
  %4 = call ptr @tspp_string_concat(ptr @9, ptr %3)
  call void @tspp_console_log(ptr %4)
  %5 = call ptr @greet(ptr @12)
  %6 = call ptr @tspp_string_concat(ptr @11, ptr %5)
  call void @tspp_console_log(ptr %6)
  %7 = call float @multiply(float 2.500000e+00, float 4.000000e+00)
  %8 = call ptr @tspp_float_to_string(float %7)
  %9 = call ptr @tspp_string_concat(ptr @13, ptr %8)
  call void @tspp_console_log(ptr %9)
  call void @tspp_free_string(ptr %8)
  %10 = call ptr @formatPoint(float 0x3FF19999A0000000, float 0.000000e+00, ptr @15)
  %11 = call ptr @tspp_string_concat(ptr @14, ptr %10)
  call void @tspp_console_log(ptr %11)
  %12 = call ptr @formatPoint(float 0x40019999A0000000, float 0x400A666660000000, ptr @17)
  %13 = call ptr @tspp_string_concat(ptr @16, ptr %12)
  call void @tspp_console_log(ptr %13)
  %14 = call i32 @square(i32 5)
  %15 = call ptr @tspp_int_to_string(i32 %14)
  %16 = call ptr @tspp_string_concat(ptr @18, ptr %15)
  call void @tspp_console_log(ptr %16)
  call void @tspp_free_string(ptr %15)
  %17 = call i32 @increment(i32 7)
  %18 = call ptr @tspp_int_to_string(i32 %17)
  %19 = call ptr @tspp_string_concat(ptr @19, ptr %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %18)
  ret i32 0
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

define i32 @main() {
entry:
  %0 = load i32, ptr @a, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call ptr @tspp_string_concat(ptr @20, ptr %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %1)
  %3 = load float, ptr @b, align 4
  %4 = call ptr @tspp_float_to_string(float %3)
  %5 = call ptr @tspp_string_concat(ptr @21, ptr %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %4)
  %6 = load i1, ptr @c, align 1
  %7 = call ptr @tspp_bool_to_string(i1 %6)
  %8 = call ptr @tspp_string_concat(ptr @22, ptr %7)
  call void @tspp_console_log(ptr %8)
  call void @tspp_free_string(ptr %7)
  %9 = load ptr, ptr @d, align 8
  %10 = call ptr @tspp_string_concat(ptr @23, ptr %9)
  call void @tspp_console_log(ptr %10)
  %11 = load i32, ptr @e, align 4
  %12 = call ptr @tspp_int_to_string(i32 %11)
  %13 = call ptr @tspp_string_concat(ptr @24, ptr %12)
  call void @tspp_console_log(ptr %13)
  call void @tspp_free_string(ptr %12)
  %14 = load float, ptr @f, align 4
  %15 = call ptr @tspp_float_to_string(float %14)
  %16 = call ptr @tspp_string_concat(ptr @25, ptr %15)
  call void @tspp_console_log(ptr %16)
  call void @tspp_free_string(ptr %15)
  %17 = load i1, ptr @g, align 1
  %18 = call ptr @tspp_bool_to_string(i1 %17)
  %19 = call ptr @tspp_string_concat(ptr @26, ptr %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %18)
  %20 = load i32, ptr @h, align 4
  %21 = call ptr @tspp_int_to_string(i32 %20)
  %22 = call ptr @tspp_string_concat(ptr @27, ptr %21)
  call void @tspp_console_log(ptr %22)
  call void @tspp_free_string(ptr %21)
  %23 = load ptr, ptr @i, align 8
  %24 = call ptr @tspp_string_concat(ptr @28, ptr %23)
  call void @tspp_console_log(ptr %24)
  %25 = load ptr, ptr @j, align 8
  %26 = call ptr @tspp_string_concat(ptr @29, ptr %25)
  call void @tspp_console_log(ptr %26)
  %27 = load ptr, ptr @k, align 8
  %28 = call ptr @tspp_ptr_to_string(ptr %27)
  %29 = call ptr @tspp_string_concat(ptr @30, ptr %28)
  call void @tspp_console_log(ptr %29)
  call void @tspp_free_string(ptr %28)
  %30 = load ptr, ptr @l, align 8
  %31 = call ptr @tspp_ptr_to_string(ptr %30)
  %32 = call ptr @tspp_string_concat(ptr @31, ptr %31)
  call void @tspp_console_log(ptr %32)
  call void @tspp_free_string(ptr %31)
  %33 = load ptr, ptr @m, align 8
  %34 = call ptr @tspp_ptr_to_string(ptr %33)
  %35 = call ptr @tspp_string_concat(ptr @32, ptr %34)
  call void @tspp_console_log(ptr %35)
  call void @tspp_free_string(ptr %34)
  %36 = load i32, ptr @n, align 4
  %37 = call ptr @tspp_int_to_string(i32 %36)
  %38 = call ptr @tspp_string_concat(ptr @33, ptr %37)
  call void @tspp_console_log(ptr %38)
  call void @tspp_free_string(ptr %37)
  %39 = load i32, ptr @o, align 4
  %40 = call ptr @tspp_int_to_string(i32 %39)
  %41 = call ptr @tspp_string_concat(ptr @34, ptr %40)
  call void @tspp_console_log(ptr %41)
  call void @tspp_free_string(ptr %40)
  %42 = load i32, ptr @p, align 4
  %43 = call ptr @tspp_int_to_string(i32 %42)
  %44 = call ptr @tspp_string_concat(ptr @35, ptr %43)
  call void @tspp_console_log(ptr %44)
  call void @tspp_free_string(ptr %43)
  %45 = load i32, ptr @q, align 4
  %46 = call ptr @tspp_int_to_string(i32 %45)
  %47 = call ptr @tspp_string_concat(ptr @36, ptr %46)
  call void @tspp_console_log(ptr %47)
  call void @tspp_free_string(ptr %46)
  %48 = load ptr, ptr @r, align 8
  %49 = call ptr @tspp_ptr_to_string(ptr %48)
  %50 = call ptr @tspp_string_concat(ptr @37, ptr %49)
  call void @tspp_console_log(ptr %50)
  call void @tspp_free_string(ptr %49)
  %51 = load i32, ptr @s, align 4
  %52 = call ptr @tspp_int_to_string(i32 %51)
  %53 = call ptr @tspp_string_concat(ptr @38, ptr %52)
  call void @tspp_console_log(ptr %53)
  call void @tspp_free_string(ptr %52)
  %54 = load i1, ptr @t, align 1
  %55 = call ptr @tspp_bool_to_string(i1 %54)
  %56 = call ptr @tspp_string_concat(ptr @39, ptr %55)
  call void @tspp_console_log(ptr %56)
  call void @tspp_free_string(ptr %55)
  %57 = call i32 @testFunctions()
  ret i32 0
}

declare ptr @tspp_bool_to_string(i1)

declare ptr @tspp_ptr_to_string(ptr)
