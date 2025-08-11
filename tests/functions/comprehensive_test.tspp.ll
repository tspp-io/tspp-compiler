; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@globalCount = internal global i32 10
@plainGlobal = global i32 3
@0 = private unnamed_addr constant [8 x i8] c"Hello, \00", align 1
@1 = private unnamed_addr constant [7 x i8] c"Vals: \00", align 1
@2 = private unnamed_addr constant [3 x i8] c", \00", align 1
@3 = private unnamed_addr constant [6 x i8] c"Ptr: \00", align 1
@4 = private unnamed_addr constant [6 x i8] c"Alpha\00", align 1
@5 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"Bob\00", align 1
@7 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@8 = private unnamed_addr constant [2 x i8] c"\22\00", align 1
@9 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @constAdd(i32 %a, i32 %b) #0 {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %0 = load i32, ptr %a1, align 4
  %1 = load i32, ptr %b2, align 4
  %addtmp = add i32 %0, %1
  ret i32 %addtmp
}

define ptr @identityString(ptr %s) {
entry:
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %0 = load ptr, ptr %s1, align 8
  ret ptr %0
}

define i32 @zeroInt() {
entry:
  ret i32 0
}

define i32 @mul(i32 %a, i32 %b) {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %0 = load i32, ptr %a1, align 4
  %1 = load i32, ptr %b2, align 4
  %multmp = mul i32 %0, %1
  ret i32 %multmp
}

define float @avg(i32 %a, i32 %b) {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %0 = load i32, ptr %a1, align 4
  %1 = load i32, ptr %b2, align 4
  %addtmp = add i32 %0, %1
  %sitofp = sitofp i32 %addtmp to float
  %fdivtmp = fdiv float %sitofp, 2.000000e+00
  ret float %fdivtmp
}

define i1 @echoBool(i1 %v) {
entry:
  %v1 = alloca i1, align 1
  store i1 %v, ptr %v1, align 1
  %0 = load i1, ptr %v1, align 1
  ret i1 %0
}

define ptr @greet(ptr %name) {
entry:
  %name1 = alloca ptr, align 8
  store ptr %name, ptr %name1, align 8
  %0 = load ptr, ptr %name1, align 8
  %1 = call ptr @tspp_string_concat(ptr @0, ptr %0)
  ret ptr %1
}

declare ptr @tspp_string_concat(ptr, ptr)

define i32 @factorial(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  %icmpeq = icmp eq i32 %0, 0
  br i1 %icmpeq, label %then, label %ifcont

then:                                             ; preds = %entry
  ret i32 1

ifcont:                                           ; preds = %entry
  %1 = load i32, ptr %n1, align 4
  %2 = load i32, ptr %n1, align 4
  %subtmp = sub i32 %2, 1
  %3 = call i32 @factorial(i32 %subtmp)
  %multmp = mul i32 %1, %3
  ret i32 %multmp
}

define ptr @getPointer() {
entry:
  %p = alloca i32, align 4
  %local = alloca i32, align 4
  store i32 42, ptr %local, align 4
  %0 = load i32, ptr %p, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  ret ptr %1
}

declare ptr @tspp_int_to_string(i32)

define ptr @buildMessage(i32 %x, i1 %y) {
entry:
  %y2 = alloca i1, align 1
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i1 %y, ptr %y2, align 1
  %0 = load i32, ptr %x1, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call ptr @tspp_string_concat(ptr @1, ptr %1)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @2)
  %4 = load i1, ptr %y2, align 1
  %5 = call ptr @tspp_bool_to_string(i1 %4)
  %6 = call ptr @tspp_string_concat(ptr %3, ptr %5)
  ret ptr %6
}

declare ptr @tspp_bool_to_string(i1)

define i32 @chainedCalls() {
entry:
  %0 = call i32 @zeroInt()
  %1 = call i32 @mul(i32 2, i32 5)
  %2 = call i32 @constAdd(i32 %0, i32 %1)
  ret i32 %2
}

define ptr @pointerString() {
entry:
  %p = alloca i32, align 4
  %0 = call ptr @getPointer()
  %1 = load i32, ptr %p, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr @3, ptr %2)
  ret ptr %3
}

define void @main_tspp() {
entry:
  %p = alloca i32, align 4
  %0 = load i32, ptr @globalCount, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = load i32, ptr @plainGlobal, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = call i32 @constAdd(i32 4, i32 6)
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = call ptr @identityString(ptr @4)
  %7 = call ptr @tspp_string_concat(ptr @5, ptr %6)
  %8 = call ptr @tspp_string_concat(ptr %7, ptr @5)
  call void @tspp_console_log(ptr %8)
  %9 = call i32 @zeroInt()
  %10 = call ptr @tspp_int_to_string(i32 %9)
  call void @tspp_console_log(ptr %10)
  call void @tspp_free_string(ptr %10)
  %11 = call i32 @mul(i32 7, i32 8)
  %12 = call ptr @tspp_int_to_string(i32 %11)
  call void @tspp_console_log(ptr %12)
  call void @tspp_free_string(ptr %12)
  %13 = call float @avg(i32 3, i32 5)
  %14 = call ptr @tspp_float_to_string(float %13)
  call void @tspp_console_log(ptr %14)
  call void @tspp_free_string(ptr %14)
  %15 = call i1 @echoBool(i1 true)
  %16 = call ptr @tspp_bool_to_string(i1 %15)
  call void @tspp_console_log(ptr %16)
  call void @tspp_free_string(ptr %16)
  %17 = call ptr @greet(ptr @6)
  %18 = call ptr @tspp_string_concat(ptr @7, ptr %17)
  %19 = call ptr @tspp_string_concat(ptr %18, ptr @7)
  call void @tspp_console_log(ptr %19)
  %20 = call i32 @factorial(i32 5)
  %21 = call ptr @tspp_int_to_string(i32 %20)
  call void @tspp_console_log(ptr %21)
  call void @tspp_free_string(ptr %21)
  %22 = call i32 @chainedCalls()
  %23 = call ptr @tspp_int_to_string(i32 %22)
  call void @tspp_console_log(ptr %23)
  call void @tspp_free_string(ptr %23)
  %24 = call ptr @buildMessage(i32 7, i1 false)
  %25 = call ptr @tspp_string_concat(ptr @8, ptr %24)
  %26 = call ptr @tspp_string_concat(ptr %25, ptr @8)
  call void @tspp_console_log(ptr %26)
  %27 = call ptr @pointerString()
  %28 = call ptr @tspp_string_concat(ptr @9, ptr %27)
  %29 = call ptr @tspp_string_concat(ptr %28, ptr @9)
  call void @tspp_console_log(ptr %29)
  %30 = call ptr @getPointer()
  %31 = load i32, ptr %p, align 4
  %32 = call ptr @tspp_int_to_string(i32 %31)
  call void @tspp_console_log(ptr %32)
  call void @tspp_free_string(ptr %32)
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_float_to_string(float)

attributes #0 = { "tspp.const" }

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
