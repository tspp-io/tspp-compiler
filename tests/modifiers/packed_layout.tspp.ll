; ModuleID = 'tspp_module'
source_filename = "tspp_module"

%P1 = type { i32, i1, i32 }
%P2 = type { i32, i1, i32 }

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define void @main_tspp() {
entry:
  %y = alloca ptr, align 8
  %x = alloca ptr, align 8
  %obj = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%P1, ptr null, i32 1) to i64))
  store ptr %obj, ptr %x, align 8
  %obj1 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%P2, ptr null, i32 1) to i64))
  store ptr %obj1, ptr %y, align 8
  %0 = load ptr, ptr %x, align 8
  %field.ptr = getelementptr %P1, ptr %0, i32 0, i32 0
  store i32 1, ptr %field.ptr, align 4
  %1 = load ptr, ptr %x, align 8
  %field.ptr2 = getelementptr %P1, ptr %1, i32 0, i32 1
  store i1 true, ptr %field.ptr2, align 1
  %2 = load ptr, ptr %x, align 8
  %field.ptr3 = getelementptr %P1, ptr %2, i32 0, i32 2
  store i32 2, ptr %field.ptr3, align 4
  %3 = load ptr, ptr %y, align 8
  %field.ptr4 = getelementptr %P2, ptr %3, i32 0, i32 0
  store i32 3, ptr %field.ptr4, align 4
  %4 = load ptr, ptr %y, align 8
  %field.ptr5 = getelementptr %P2, ptr %4, i32 0, i32 1
  store i1 false, ptr %field.ptr5, align 1
  %5 = load ptr, ptr %y, align 8
  %field.ptr6 = getelementptr %P2, ptr %5, i32 0, i32 2
  store i32 4, ptr %field.ptr6, align 4
  ret void
}

declare ptr @GC_malloc(i64)

define i32 @main() {
entry:
  call void @main_tspp()
  ret i32 0
}
