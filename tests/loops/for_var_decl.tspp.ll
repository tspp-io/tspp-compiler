; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @main() {
entry:
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %icmpolt = icmp slt i32 %0, 3
  br i1 %icmpolt, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %1 = load i32, ptr %i, align 4
  %addtmp = add i32 %1, 1
  store i32 %addtmp, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 0
}
