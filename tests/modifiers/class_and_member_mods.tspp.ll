; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__tspp_gc_ctor, ptr null }]

declare void @GC_init()

define internal void @__tspp_gc_ctor() {
entry:
  call void @GC_init()
  ret void
}

define void @M.f1() #0 {
entry:
  ret void
}

define void @M.f2(ptr %this) #1 {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

define void @M.f3(ptr %this) #2 {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

define void @M.f4(ptr %this) #3 {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

attributes #0 = { "tspp.static" }
attributes #1 = { "tspp.inline" "tspp.virtual" }
attributes #2 = { "tspp.override" }
attributes #3 = { "tspp.abstract" }

define i32 @main() {
entry:
  ret i32 0
}
