; ModuleID = 'tspp_module'
source_filename = "tspp_module"

@x = global i32 10
@y = global i32 20
@sum = global i32 0
@product = global i32 0
@str = global ptr @0
@0 = private constant [8 x i8] c"\22Hello\22\00"

define i1 @main() {
entry:
  %0 = load i32, ptr @x, align 4
  %1 = load i32, ptr @y, align 4
  %addtmp = add i32 %0, %1
  store i32 %addtmp, ptr @sum, align 4
  %2 = load i32, ptr @x, align 4
  %3 = load i32, ptr @y, align 4
  %multmp = mul i32 %2, %3
  store i32 %multmp, ptr @product, align 4
  %4 = load i32, ptr @sum, align 4
  %5 = call ptr @tspp_int_to_string(i32 %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %5)
  %6 = load i32, ptr @product, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  call void @tspp_console_log(ptr %7)
  call void @tspp_free_string(ptr %7)
  %8 = load ptr, ptr @str, align 8
  call void @tspp_console_log(ptr %8)
  ret i1 false
}

declare ptr @tspp_int_to_string(i32)

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)
