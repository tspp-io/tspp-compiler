; ModuleID = 'tspp_module'
source_filename = "tspp_module"
target triple = "x86_64-pc-linux-gnu"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@f = global ptr 0

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

declare ptr @GC_malloc(i64)
