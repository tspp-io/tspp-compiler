; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@a = global i32 42
@pa = global ptr null
@0 = private unnamed_addr constant [24 x i8] c"Testing simple pointers\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  store ptr @a, ptr @pa, align 8
  ret void
}

define i1 @test() {
entry:
  %0 = call ptr @tspp_string_concat(ptr @1, ptr @0)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @1)
  call void @tspp_console_log(ptr %1)
  %2 = load i32, ptr @a, align 4
  %3 = call ptr @tspp_int_to_string(i32 %2)
  call void @tspp_console_log(ptr %3)
  call void @tspp_free_string(ptr %3)
  %4 = load ptr, ptr @pa, align 8
  %deref = load i32, ptr %4, align 4
  %5 = call ptr @tspp_int_to_string(i32 %deref)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  ret i1 true
}

declare ptr @tspp_string_concat(ptr, ptr)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

declare ptr @tspp_int_to_string(i32)

define i1 @main() {
entry:
  %0 = call i1 @test()
  ret i1 %0
}
