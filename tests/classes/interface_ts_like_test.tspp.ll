; ModuleID = 'tspp_module'
source_filename = "tspp_module"

%Impl = type { i32, i32 }

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define void @__ctor_Impl(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

define i32 @Impl.getId(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Impl, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  ret i32 %1
}

define void @Impl.setValue(ptr %this, i32 %v) {
entry:
  %v2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %v, ptr %v2, align 4
  %0 = load i32, ptr %v2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Impl, ptr %1, i32 0, i32 1
  store i32 %0, ptr %field.ptr, align 4
  ret void
}

define i32 @main() {
entry:
  ret i32 0
}
