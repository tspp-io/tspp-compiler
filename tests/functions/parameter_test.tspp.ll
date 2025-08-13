; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [6 x i8] c"param\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @sum(i32 %a, i32 %b) {
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

define i1 @andOp(i1 %a, i1 %b) {
entry:
  %b2 = alloca i1, align 1
  %a1 = alloca i1, align 1
  store i1 %a, ptr %a1, align 1
  store i1 %b, ptr %b2, align 1
  %0 = load i1, ptr %a1, align 1
  br i1 %0, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %1 = load i1, ptr %b2, align 1
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %1, %land.rhs ]
  ret i1 %land
}

define float @average(float %a, float %b, float %c) {
entry:
  %c3 = alloca float, align 4
  %b2 = alloca float, align 4
  %a1 = alloca float, align 4
  store float %a, ptr %a1, align 4
  store float %b, ptr %b2, align 4
  store float %c, ptr %c3, align 4
  %0 = load float, ptr %a1, align 4
  %1 = load float, ptr %b2, align 4
  %faddtmp = fadd float %0, %1
  %2 = load float, ptr %c3, align 4
  %faddtmp4 = fadd float %faddtmp, %2
  %fdivtmp = fdiv float %faddtmp4, 3.000000e+00
  ret float %fdivtmp
}

define ptr @echo(ptr %s) {
entry:
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %0 = load ptr, ptr %s1, align 8
  ret ptr %0
}

define ptr @idPtr(ptr %p) {
entry:
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  %0 = load ptr, ptr %p1, align 8
  ret ptr %0
}

define void @main_tspp() {
entry:
  %value = alloca i32, align 4
  %0 = call i32 @sum(i32 5, i32 7)
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = call i1 @andOp(i1 true, i1 false)
  %3 = call ptr @tspp_bool_to_string(i1 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = call float @average(float 1.000000e+00, float 2.000000e+00, float 3.000000e+00)
  %5 = call ptr @tspp_float_to_string(float %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = call ptr @echo(ptr @0)
  %7 = call ptr @tspp_string_concat(ptr @1, ptr %6)
  %8 = call ptr @tspp_string_concat(ptr %7, ptr @1)
  call void @tspp_console_log(ptr %8)
  store i32 42, ptr %value, align 4
  %9 = call ptr @idPtr(ptr %value)
  %10 = call ptr @tspp_ptr_to_string(ptr %9)
  call void @tspp_console_log(ptr %10)
  call void @tspp_free_string(ptr %10)
  ret void
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_bool_to_string(i1)

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_string_concat(ptr, ptr)

declare ptr @tspp_ptr_to_string(ptr)

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
