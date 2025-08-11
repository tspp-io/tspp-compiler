; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [3 x i8] c"\22\22\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"\22hi\22\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"\22s\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @choose(i1 %cond, i32 %a, i32 %b) {
entry:
  %b3 = alloca i32, align 4
  %a2 = alloca i32, align 4
  %cond1 = alloca i1, align 1
  store i1 %cond, ptr %cond1, align 1
  store i32 %a, ptr %a2, align 4
  store i32 %b, ptr %b3, align 4
  %0 = load i1, ptr %cond1, align 1
  br i1 %0, label %then, label %ifcont

then:                                             ; preds = %entry
  %1 = load i32, ptr %a2, align 4
  ret i32 %1

ifcont:                                           ; preds = %entry
  %2 = load i32, ptr %b3, align 4
  ret i32 %2
}

define float @mix(i32 %x, float %y) {
entry:
  %y2 = alloca float, align 4
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store float %y, ptr %y2, align 4
  %0 = load i32, ptr %x1, align 4
  %1 = load float, ptr %y2, align 4
  %sitofp = sitofp i32 %0 to float
  %faddtmp = fadd float %sitofp, %1
  ret float %faddtmp
}

define ptr @greetNTimes(ptr %who, i32 %n) {
entry:
  %i = alloca i32, align 4
  %n2 = alloca i32, align 4
  %who1 = alloca ptr, align 8
  store ptr %who, ptr %who1, align 8
  store i32 %n, ptr %n2, align 4
  %acc = call ptr @GC_malloc(i64 8)
  store ptr @0, ptr %acc, align 8
  store i32 0, ptr %i, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %n2, align 4
  %icmpolt = icmp slt i32 %0, %1
  br i1 %icmpolt, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %2 = load ptr, ptr %acc, align 8
  %3 = load ptr, ptr %who1, align 8
  %4 = call ptr @tspp_string_concat(ptr %2, ptr %3)
  store ptr %4, ptr %acc, align 8
  %5 = load i32, ptr %i, align 4
  %addtmp = add i32 %5, 1
  store i32 %addtmp, ptr %i, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %6 = load ptr, ptr %acc, align 8
  ret ptr %6
}

declare ptr @GC_malloc(i64)

declare ptr @tspp_string_concat(ptr, ptr)

define ptr @unionPick(i1 %cond, i32 %a, ptr %b) {
entry:
  %b3 = alloca ptr, align 8
  %a2 = alloca i32, align 4
  %cond1 = alloca i1, align 1
  store i1 %cond, ptr %cond1, align 1
  store i32 %a, ptr %a2, align 4
  store ptr %b, ptr %b3, align 8
  %0 = load i1, ptr %cond1, align 1
  br i1 %0, label %then, label %ifcont

then:                                             ; preds = %entry
  %1 = load i32, ptr %a2, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  ret ptr %2

ifcont:                                           ; preds = %entry
  %3 = load ptr, ptr %b3, align 8
  ret ptr %3
}

declare ptr @tspp_int_to_string(i32)

define void @main_tspp() {
entry:
  %p = alloca i32, align 4
  %v = alloca i32, align 4
  %0 = call i32 @choose(i1 true, i32 3, i32 9)
  %1 = call ptr @tspp_int_to_string(i32 %0)
  call void @tspp_console_log(ptr %1)
  call void @tspp_free_string(ptr %1)
  %2 = call float @mix(i32 2, float 2.500000e+00)
  %3 = call ptr @tspp_float_to_string(float %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = call ptr @greetNTimes(ptr @1, i32 3)
  call void @tspp_console_log(ptr %4)
  %5 = call ptr @unionPick(i1 false, i32 7, ptr @2)
  call void @tspp_console_log(ptr %5)
  store i32 10, ptr %v, align 4
  %6 = load i32, ptr %p, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  call void @tspp_console_log(ptr %7)
  call void @tspp_free_string(ptr %7)
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_float_to_string(float)

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
