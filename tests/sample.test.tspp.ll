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
@i = global i32 0
@j = global i32 0
@k = global ptr null
@l = global i32 0
@m = global i32 0
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

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  store ptr null, ptr @r, align 8
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
  %22 = load i32, ptr @i, align 4
  %23 = call ptr @tspp_int_to_string(i32 %22)
  %24 = call ptr @tspp_string_concat(ptr @12, ptr %23)
  call void @tspp_console_log(ptr %24)
  call void @tspp_free_string(ptr %23)
  %25 = load i32, ptr @j, align 4
  %26 = call ptr @tspp_int_to_string(i32 %25)
  %27 = call ptr @tspp_string_concat(ptr @13, ptr %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %26)
  %28 = load ptr, ptr @k, align 8
  %29 = call ptr @tspp_ptr_to_string(ptr %28)
  %30 = call ptr @tspp_string_concat(ptr @14, ptr %29)
  call void @tspp_console_log(ptr %30)
  call void @tspp_free_string(ptr %29)
  %31 = load i32, ptr @l, align 4
  %32 = call ptr @tspp_int_to_string(i32 %31)
  %33 = call ptr @tspp_string_concat(ptr @15, ptr %32)
  call void @tspp_console_log(ptr %33)
  call void @tspp_free_string(ptr %32)
  %34 = load i32, ptr @m, align 4
  %35 = call ptr @tspp_int_to_string(i32 %34)
  %36 = call ptr @tspp_string_concat(ptr @16, ptr %35)
  call void @tspp_console_log(ptr %36)
  call void @tspp_free_string(ptr %35)
  %37 = load i32, ptr @n, align 4
  %38 = call ptr @tspp_int_to_string(i32 %37)
  %39 = call ptr @tspp_string_concat(ptr @17, ptr %38)
  call void @tspp_console_log(ptr %39)
  call void @tspp_free_string(ptr %38)
  %40 = load i32, ptr @o, align 4
  %41 = call ptr @tspp_int_to_string(i32 %40)
  %42 = call ptr @tspp_string_concat(ptr @18, ptr %41)
  call void @tspp_console_log(ptr %42)
  call void @tspp_free_string(ptr %41)
  %43 = load i32, ptr @p, align 4
  %44 = call ptr @tspp_int_to_string(i32 %43)
  %45 = call ptr @tspp_string_concat(ptr @19, ptr %44)
  call void @tspp_console_log(ptr %45)
  call void @tspp_free_string(ptr %44)
  %46 = load i32, ptr @q, align 4
  %47 = call ptr @tspp_int_to_string(i32 %46)
  %48 = call ptr @tspp_string_concat(ptr @20, ptr %47)
  call void @tspp_console_log(ptr %48)
  call void @tspp_free_string(ptr %47)
  %49 = load ptr, ptr @r, align 8
  %50 = call ptr @tspp_string_concat(ptr @21, ptr %49)
  call void @tspp_console_log(ptr %50)
  %51 = load float, ptr @s, align 4
  %52 = call ptr @tspp_float_to_string(float %51)
  %53 = call ptr @tspp_string_concat(ptr @22, ptr %52)
  call void @tspp_console_log(ptr %53)
  call void @tspp_free_string(ptr %52)
  %54 = load i1, ptr @t, align 1
  %55 = call ptr @tspp_bool_to_string(i1 %54)
  %56 = call ptr @tspp_string_concat(ptr @23, ptr %55)
  call void @tspp_console_log(ptr %56)
  call void @tspp_free_string(ptr %55)
  ret i32 0
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_bool_to_string(i1)

declare ptr @tspp_ptr_to_string(ptr)
