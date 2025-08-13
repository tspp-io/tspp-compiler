; ModuleID = 'tspp_module'
source_filename = "tspp_module"

%Greeter = type { ptr }

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]
@0 = private unnamed_addr constant [8 x i8] c"Hello, \00", align 1
@1 = private unnamed_addr constant [9 x i8] c"compiled\00", align 1
@2 = private unnamed_addr constant [2 x i8] c"\22\00", align 1

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define void @__ctor_Greeter(ptr %this, ptr %name) {
entry:
  %name2 = alloca ptr, align 8
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store ptr %name, ptr %name2, align 8
  %0 = load ptr, ptr %name2, align 8
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Greeter, ptr %1, i32 0, i32 0
  store ptr %0, ptr %field.ptr, align 8
  ret void
}

define ptr @Greeter.greet(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Greeter, ptr %0, i32 0, i32 0
  %1 = load ptr, ptr %field.ptr, align 8
  %2 = call ptr @tspp_ptr_to_string(ptr %1)
  %3 = call ptr @tspp_string_concat(ptr @0, ptr %2)
  ret ptr %3
}

declare ptr @tspp_ptr_to_string(ptr)

declare ptr @tspp_string_concat(ptr, ptr)

define i32 @main() {
entry:
  %0 = call ptr @tspp_string_concat(ptr @2, ptr @1)
  %1 = call ptr @tspp_string_concat(ptr %0, ptr @2)
  call void @tspp_console_log(ptr %1)
  ret i32 0
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
