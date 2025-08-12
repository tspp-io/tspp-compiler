; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define i32 @Base_baseVal(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret i32 0
}

define i32 @Derived_derivedSum(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret i32 0
}

define i32 @main() {
entry:
  ret i32 0
}
