; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @Point_sum(ptr %this, i32 %a, i32 %b) {
entry:
  %b3 = alloca i32, align 4
  %a2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %a, ptr %a2, align 4
  store i32 %b, ptr %b3, align 4
  %0 = load i32, ptr %a2, align 4
  %1 = load i32, ptr %b3, align 4
  %addtmp = add i32 %0, %1
  ret i32 %addtmp
}

define i32 @Point_selfSum(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret i32 0
}

define i32 @main() {
entry:
  ret i32 0
}
