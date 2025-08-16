; ModuleID = 'tspp_module'
source_filename = "tspp_module"

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
@h = global ptr @3
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
@s = global float 5.500000e+00
@t = global i1 false
@4 = private unnamed_addr constant [4 x i8] c"a: \00", align 1
@5 = private unnamed_addr constant [4 x i8] c"b: \00", align 1
@6 = private unnamed_addr constant [4 x i8] c"c: \00", align 1
@7 = private unnamed_addr constant [4 x i8] c"d: \00", align 1
@8 = private unnamed_addr constant [4 x i8] c"e: \00", align 1
@9 = private unnamed_addr constant [4 x i8] c"f: \00", align 1
@10 = private unnamed_addr constant [4 x i8] c"g: \00", align 1
@11 = private unnamed_addr constant [4 x i8] c"h: \00", align 1
@12 = private unnamed_addr constant [4 x i8] c"i: \00", align 1
@13 = private unnamed_addr constant [4 x i8] c"j: \00", align 1
@14 = private unnamed_addr constant [4 x i8] c"k: \00", align 1
@15 = private unnamed_addr constant [4 x i8] c"l: \00", align 1
@16 = private unnamed_addr constant [4 x i8] c"m: \00", align 1
@17 = private unnamed_addr constant [4 x i8] c"n: \00", align 1
@18 = private unnamed_addr constant [4 x i8] c"o: \00", align 1
@19 = private unnamed_addr constant [4 x i8] c"p: \00", align 1
@20 = private unnamed_addr constant [4 x i8] c"q: \00", align 1
@21 = private unnamed_addr constant [4 x i8] c"r: \00", align 1
@22 = private unnamed_addr constant [4 x i8] c"s: \00", align 1
@23 = private unnamed_addr constant [4 x i8] c"t: \00", align 1
@24 = private unnamed_addr constant [3 x i8] c"{ \00", align 1
@25 = private unnamed_addr constant [5 x i8] c"name\00", align 1
@26 = private unnamed_addr constant [3 x i8] c": \00", align 1
@27 = private unnamed_addr constant [5 x i8] c"John\00", align 1
@28 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@29 = private unnamed_addr constant [3 x i8] c", \00", align 1
@30 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@31 = private unnamed_addr constant [3 x i8] c": \00", align 1
@32 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@33 = private unnamed_addr constant [3 x i8] c"{ \00", align 1
@34 = private unnamed_addr constant [5 x i8] c"unit\00", align 1
@35 = private unnamed_addr constant [3 x i8] c": \00", align 1
@36 = private unnamed_addr constant [2 x i8] c"m\00", align 1
@37 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@38 = private unnamed_addr constant [3 x i8] c", \00", align 1
@39 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@40 = private unnamed_addr constant [3 x i8] c": \00", align 1
@41 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@42 = private unnamed_addr constant [3 x i8] c"{ \00", align 1
@43 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@44 = private unnamed_addr constant [3 x i8] c": \00", align 1
@45 = private unnamed_addr constant [3 x i8] c", \00", align 1
@46 = private unnamed_addr constant [2 x i8] c"y\00", align 1
@47 = private unnamed_addr constant [3 x i8] c": \00", align 1
@48 = private unnamed_addr constant [3 x i8] c" }\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  %0 = call ptr @tspp_string_concat(ptr @24, ptr @25)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @26)
  %2 = call ptr @tspp_string_concat(ptr %1, ptr @28)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @27)
  %4 = call ptr @tspp_string_concat(ptr %3, ptr @28)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @29)
  %6 = call ptr @tspp_string_concat(ptr %5, ptr @30)
  %7 = call ptr @tspp_string_concat(ptr %6, ptr @31)
  %8 = call ptr @tspp_int_to_string(i32 1)
  %9 = call ptr @tspp_string_concat(ptr %7, ptr %8)
  %10 = call ptr @tspp_string_concat(ptr %9, ptr @32)
  store ptr %10, ptr @i, align 8
  %11 = call ptr @tspp_string_concat(ptr @33, ptr @34)
  %12 = call ptr @tspp_string_concat(ptr %11, ptr @35)
  %13 = call ptr @tspp_string_concat(ptr %12, ptr @37)
  %14 = call ptr @tspp_string_concat(ptr %13, ptr @36)
  %15 = call ptr @tspp_string_concat(ptr %14, ptr @37)
  %16 = call ptr @tspp_string_concat(ptr %15, ptr @38)
  %17 = call ptr @tspp_string_concat(ptr %16, ptr @39)
  %18 = call ptr @tspp_string_concat(ptr %17, ptr @40)
  %19 = call ptr @tspp_float_to_string(float 5.500000e+00)
  %20 = call ptr @tspp_string_concat(ptr %18, ptr %19)
  %21 = call ptr @tspp_string_concat(ptr %20, ptr @41)
  store ptr %21, ptr @j, align 8
  %22 = call ptr @tspp_string_concat(ptr @42, ptr @43)
  %23 = call ptr @tspp_string_concat(ptr %22, ptr @44)
  %24 = call ptr @tspp_float_to_string(float 0x3FF19999A0000000)
  %25 = call ptr @tspp_string_concat(ptr %23, ptr %24)
  %26 = call ptr @tspp_string_concat(ptr %25, ptr @45)
  %27 = call ptr @tspp_string_concat(ptr %26, ptr @46)
  %28 = call ptr @tspp_string_concat(ptr %27, ptr @47)
  %29 = call ptr @tspp_float_to_string(float 0x40019999A0000000)
  %30 = call ptr @tspp_string_concat(ptr %28, ptr %29)
  %31 = call ptr @tspp_string_concat(ptr %30, ptr @48)
  store ptr %31, ptr @r, align 8
  ret void
}

define i32 @main() {
entry:
  %0 = load i32, ptr @a, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call ptr @tspp_string_concat(ptr @4, ptr %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %1)
  %3 = load float, ptr @b, align 4
  %4 = call ptr @tspp_float_to_string(float %3)
  %5 = call ptr @tspp_string_concat(ptr @5, ptr %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %4)
  %6 = load i1, ptr @c, align 1
  %7 = call ptr @tspp_bool_to_string(i1 %6)
  %8 = call ptr @tspp_string_concat(ptr @6, ptr %7)
  call void @tspp_console_log(ptr %8)
  call void @tspp_free_string(ptr %7)
  %9 = load ptr, ptr @d, align 8
  %10 = call ptr @tspp_string_concat(ptr @7, ptr %9)
  call void @tspp_console_log(ptr %10)
  %11 = load i32, ptr @e, align 4
  %12 = call ptr @tspp_int_to_string(i32 %11)
  %13 = call ptr @tspp_string_concat(ptr @8, ptr %12)
  call void @tspp_console_log(ptr %13)
  call void @tspp_free_string(ptr %12)
  %14 = load float, ptr @f, align 4
  %15 = call ptr @tspp_float_to_string(float %14)
  %16 = call ptr @tspp_string_concat(ptr @9, ptr %15)
  call void @tspp_console_log(ptr %16)
  call void @tspp_free_string(ptr %15)
  %17 = load i1, ptr @g, align 1
  %18 = call ptr @tspp_bool_to_string(i1 %17)
  %19 = call ptr @tspp_string_concat(ptr @10, ptr %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %18)
  %20 = load ptr, ptr @h, align 8
  %21 = call ptr @tspp_string_concat(ptr @11, ptr %20)
  call void @tspp_console_log(ptr %21)
  %22 = load ptr, ptr @i, align 8
  %23 = call ptr @tspp_string_concat(ptr @12, ptr %22)
  call void @tspp_console_log(ptr %23)
  %24 = load ptr, ptr @j, align 8
  %25 = call ptr @tspp_string_concat(ptr @13, ptr %24)
  call void @tspp_console_log(ptr %25)
  %26 = load ptr, ptr @k, align 8
  %27 = call ptr @tspp_ptr_to_string(ptr %26)
  %28 = call ptr @tspp_string_concat(ptr @14, ptr %27)
  call void @tspp_console_log(ptr %28)
  call void @tspp_free_string(ptr %27)
  %29 = load ptr, ptr @l, align 8
  %30 = call ptr @tspp_ptr_to_string(ptr %29)
  %31 = call ptr @tspp_string_concat(ptr @15, ptr %30)
  call void @tspp_console_log(ptr %31)
  call void @tspp_free_string(ptr %30)
  %32 = load ptr, ptr @m, align 8
  %33 = call ptr @tspp_ptr_to_string(ptr %32)
  %34 = call ptr @tspp_string_concat(ptr @16, ptr %33)
  call void @tspp_console_log(ptr %34)
  call void @tspp_free_string(ptr %33)
  %35 = load i32, ptr @n, align 4
  %36 = call ptr @tspp_int_to_string(i32 %35)
  %37 = call ptr @tspp_string_concat(ptr @17, ptr %36)
  call void @tspp_console_log(ptr %37)
  call void @tspp_free_string(ptr %36)
  %38 = load i32, ptr @o, align 4
  %39 = call ptr @tspp_int_to_string(i32 %38)
  %40 = call ptr @tspp_string_concat(ptr @18, ptr %39)
  call void @tspp_console_log(ptr %40)
  call void @tspp_free_string(ptr %39)
  %41 = load i32, ptr @p, align 4
  %42 = call ptr @tspp_int_to_string(i32 %41)
  %43 = call ptr @tspp_string_concat(ptr @19, ptr %42)
  call void @tspp_console_log(ptr %43)
  call void @tspp_free_string(ptr %42)
  %44 = load i32, ptr @q, align 4
  %45 = call ptr @tspp_int_to_string(i32 %44)
  %46 = call ptr @tspp_string_concat(ptr @20, ptr %45)
  call void @tspp_console_log(ptr %46)
  call void @tspp_free_string(ptr %45)
  %47 = load ptr, ptr @r, align 8
  %48 = call ptr @tspp_string_concat(ptr @21, ptr %47)
  call void @tspp_console_log(ptr %48)
  %49 = load float, ptr @s, align 4
  %50 = call ptr @tspp_float_to_string(float %49)
  %51 = call ptr @tspp_string_concat(ptr @22, ptr %50)
  call void @tspp_console_log(ptr %51)
  call void @tspp_free_string(ptr %50)
  %52 = load i1, ptr @t, align 1
  %53 = call ptr @tspp_bool_to_string(i1 %52)
  %54 = call ptr @tspp_string_concat(ptr @23, ptr %53)
  call void @tspp_console_log(ptr %54)
  call void @tspp_free_string(ptr %53)
  ret i32 0
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_bool_to_string(i1)

declare ptr @tspp_ptr_to_string(ptr)
