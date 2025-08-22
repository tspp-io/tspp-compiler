; ModuleID = 'tspp'
source_filename = "tspp"

%Point = type { i32, i32, i32, i32, i32 }
%ByteArray = type <{ i32 }>
%ResourceHandle = type { i32 }
%Animal = type { i32 }
%Dog = type { i32, i32 }
%Circle = type { i32 }
%SBox = type { i32 }
%PAdapter = type { i32, i32 }

@Point.createdCount = internal global i32 0
@0 = private unnamed_addr constant [19 x i8] c"Point.static block\00", align 1
@1 = private unnamed_addr constant [13 x i8] c"Point.count=\00", align 1
@2 = private unnamed_addr constant [2 x i8] c"(\00", align 1
@3 = private unnamed_addr constant [3 x i8] c", \00", align 1
@4 = private unnamed_addr constant [2 x i8] c")\00", align 1
@5 = private unnamed_addr constant [10 x i8] c"Resource#\00", align 1
@Animal.kingdomName = internal global i32 0
@6 = private unnamed_addr constant [15 x i8] c" makes a sound\00", align 1
@7 = private unnamed_addr constant [9 x i8] c" barks (\00", align 1
@8 = private unnamed_addr constant [2 x i8] c")\00", align 1
@9 = private unnamed_addr constant [2 x i8] c"(\00", align 1
@10 = private unnamed_addr constant [3 x i8] c", \00", align 1
@11 = private unnamed_addr constant [2 x i8] c")\00", align 1
@12 = private unnamed_addr constant [9 x i8] c"Adapter:\00", align 1
@13 = private unnamed_addr constant [6 x i8] c"Point\00", align 1
@14 = private unnamed_addr constant [2 x i8] c"!\00", align 1
@15 = private unnamed_addr constant [6 x i8] c"Desc=\00", align 1
@16 = private unnamed_addr constant [3 x i8] c"ok\00", align 1
@17 = private unnamed_addr constant [7 x i8] c"dist: \00", align 1
@18 = private unnamed_addr constant [8 x i8] c"ident: \00", align 1
@19 = private unnamed_addr constant [11 x i8] c"flexible: \00", align 1
@20 = private unnamed_addr constant [5 x i8] c"p1: \00", align 1
@21 = private unnamed_addr constant [11 x i8] c"p1 moved: \00", align 1
@22 = private unnamed_addr constant [21 x i8] c"stringifyPoint(p1): \00", align 1
@23 = private unnamed_addr constant [3 x i8] c"hi\00", align 1
@24 = private unnamed_addr constant [19 x i8] c"useBoxString(sb): \00", align 1
@25 = private unnamed_addr constant [8 x i8] c"Generic\00", align 1
@26 = private unnamed_addr constant [12 x i8] c"a.speak(): \00", align 1
@27 = private unnamed_addr constant [19 x i8] c"Animal.kingdom(): \00", align 1
@28 = private unnamed_addr constant [4 x i8] c"Rex\00", align 1
@29 = private unnamed_addr constant [9 x i8] c"Labrador\00", align 1
@30 = private unnamed_addr constant [12 x i8] c"d.speak(): \00", align 1
@31 = private unnamed_addr constant [15 x i8] c"d.getBreed(): \00", align 1
@32 = private unnamed_addr constant [11 x i8] c"c.area(): \00", align 1
@33 = private unnamed_addr constant [12 x i8] c"ba.get(0): \00", align 1
@34 = private unnamed_addr constant [22 x i8] c"firstOf<int>(10,20): \00", align 1
@35 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@36 = private unnamed_addr constant [2 x i8] c"A\00", align 1
@37 = private unnamed_addr constant [12 x i8] c"bxS.get(): \00", align 1
@38 = private unnamed_addr constant [12 x i8] c"bxI.get(): \00", align 1
@39 = private unnamed_addr constant [12 x i8] c"isEven(6): \00", align 1
@40 = private unnamed_addr constant [11 x i8] c"sumTo(5): \00", align 1
@41 = private unnamed_addr constant [16 x i8] c"forVariants(): \00", align 1
@42 = private unnamed_addr constant [19 x i8] c"precedenceDemo(): \00", align 1
@43 = private unnamed_addr constant [25 x i8] c"memberAndIndexAccess(): \00", align 1
@44 = private unnamed_addr constant [4 x i8] c"x: \00", align 1
@45 = private unnamed_addr constant [4 x i8] c"y: \00", align 1
@46 = private unnamed_addr constant [10 x i8] c"logical: \00", align 1
@47 = private unnamed_addr constant [7 x i8] c"1<<3: \00", align 1
@48 = private unnamed_addr constant [7 x i8] c"8>>1: \00", align 1
@49 = private unnamed_addr constant [5 x i8] c"~0: \00", align 1
@50 = private unnamed_addr constant [16 x i8] c"pointerPlay(): \00", align 1
@51 = private unnamed_addr constant [16 x i8] c"storagePlay(): \00", align 1

define internal void @__tspp_gc_ctor() {
entry:
  call void @tspp_console_log(ptr @0)
  ret void
}

declare void @tspp_console_log(ptr)

declare void @tspp_free_string(ptr)

define void @__ctor_Point(ptr %this, i32 %x, i32 %y) {
entry:
  %y3 = alloca i32, align 4
  %x2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %x, ptr %x2, align 4
  store i32 %y, ptr %y3, align 4
  %0 = load i32, ptr %x2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Point, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  %2 = load i32, ptr %y3, align 4
  %3 = load ptr, ptr %this1, align 8
  %field.ptr4 = getelementptr %Point, ptr %3, i32 0, i32 1
  store i32 %2, ptr %field.ptr4, align 4
  %4 = load i32, ptr @Point.createdCount, align 4
  %addtmp = add i32 %4, 1
  store i32 %addtmp, ptr @Point.createdCount, align 4
  ret void
}

define i32 @Point.stats() #0 {
entry:
  %0 = load i32, ptr @Point.createdCount, align 4
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call ptr @tspp_string_concat(ptr @1, ptr %1)
  ret ptr %2
}

declare ptr @tspp_int_to_string(i32)

declare ptr @tspp_string_concat(ptr, ptr)

define i32 @Point.toString(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Point, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr @2, ptr %2)
  %4 = call ptr @tspp_string_concat(ptr %3, ptr @3)
  %5 = load ptr, ptr %this1, align 8
  %field.ptr2 = getelementptr %Point, ptr %5, i32 0, i32 1
  %6 = load i32, ptr %field.ptr2, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  %8 = call ptr @tspp_string_concat(ptr %4, ptr %7)
  %9 = call ptr @tspp_string_concat(ptr %8, ptr @4)
  ret ptr %9
}

define i32 @Point.move(ptr %this, i32 %dx, i32 %dy) {
entry:
  %dy3 = alloca i32, align 4
  %dx2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %dx, ptr %dx2, align 4
  store i32 %dy, ptr %dy3, align 4
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Point, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = load i32, ptr %dx2, align 4
  %addtmp = add i32 %1, %2
  %3 = load ptr, ptr %this1, align 8
  %field.ptr4 = getelementptr %Point, ptr %3, i32 0, i32 0
  store i32 %addtmp, ptr %field.ptr4, align 4
  %4 = load ptr, ptr %this1, align 8
  %field.ptr5 = getelementptr %Point, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %field.ptr5, align 4
  %6 = load i32, ptr %dy3, align 4
  %addtmp6 = add i32 %5, %6
  %7 = load ptr, ptr %this1, align 8
  %field.ptr7 = getelementptr %Point, ptr %7, i32 0, i32 1
  store i32 %addtmp6, ptr %field.ptr7, align 4
  ret i32 0
}

define void @__ctor_ByteArray(ptr %this, i32 %n) {
entry:
  %n2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %n, ptr %n2, align 4
  %0 = load i32, ptr %n2, align 4
  %1 = mul i32 %0, 4
  %2 = sext i32 %1 to i64
  %arr = call ptr @GC_malloc(i64 %2)
  %3 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %ByteArray, ptr %3, i32 0, i32 0
  store ptr %arr, ptr %field.ptr, align 8
  ret void
}

declare ptr @GC_malloc(i64)

define i32 @ByteArray.set(ptr %this, i32 %i, i32 %v) {
entry:
  %v3 = alloca i32, align 4
  %i2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %i, ptr %i2, align 4
  store i32 %v, ptr %v3, align 4
  %0 = load i32, ptr %v3, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %ByteArray, ptr %1, i32 0, i32 0
  %2 = load i32, ptr %field.ptr, align 4
  %3 = load i32, ptr %i2, align 4
  %4 = inttoptr i32 %2 to ptr
  %elem.ptr = getelementptr i32, ptr %4, i32 %3
  store i32 %0, ptr %elem.ptr, align 4
  ret i32 0
}

define i32 @ByteArray.get(ptr %this, i32 %i) {
entry:
  %i2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %i, ptr %i2, align 4
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %ByteArray, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = load i32, ptr %i2, align 4
  %3 = inttoptr i32 %1 to ptr
  %elem.ptr = getelementptr i32, ptr %3, i32 %2
  %4 = load i32, ptr %elem.ptr, align 4
  ret i32 %4
}

define void @__ctor_ResourceHandle(ptr %this, i32 %id) {
entry:
  %id2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %id, ptr %id2, align 4
  %0 = load i32, ptr %id2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %ResourceHandle, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  ret void
}

define i32 @ResourceHandle.print(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %ResourceHandle, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr @5, ptr %2)
  call void @tspp_console_log(ptr %3)
  ret i32 0
}

define void @__ctor_Animal(ptr %this, i32 %name) {
entry:
  %name2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %name, ptr %name2, align 4
  %0 = load i32, ptr %name2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Animal, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  ret void
}

define i32 @Animal.speak(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Animal, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @6)
  ret ptr %3
}

define i32 @Animal.kingdom() #0 {
entry:
  %0 = load i32, ptr @Animal.kingdomName, align 4
  ret i32 %0
}

define void @__ctor_Dog(ptr %this, i32 %name, i32 %breed) {
entry:
  %breed3 = alloca i32, align 4
  %name2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %name, ptr %name2, align 4
  store i32 %breed, ptr %breed3, align 4
  %0 = load ptr, ptr %this1, align 8
  %1 = load i32, ptr %name2, align 4
  call void @__ctor_Animal(ptr %0, i32 %1)
  %2 = load i32, ptr %breed3, align 4
  %3 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Dog, ptr %3, i32 0, i32 1
  store i32 %2, ptr %field.ptr, align 4
  ret void
}

define i32 @Dog.speak(ptr %this) #1 {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Dog, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr %2, ptr @7)
  %4 = load ptr, ptr %this1, align 8
  %field.ptr2 = getelementptr %Dog, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %field.ptr2, align 4
  %6 = call ptr @tspp_int_to_string(i32 %5)
  %7 = call ptr @tspp_string_concat(ptr %3, ptr %6)
  %8 = call ptr @tspp_string_concat(ptr %7, ptr @8)
  ret ptr %8
}

define i32 @Dog.getBreed(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Dog, ptr %0, i32 0, i32 1
  %1 = load i32, ptr %field.ptr, align 4
  ret i32 %1
}

define i32 @Dog.print(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %1 = call i32 @Dog.speak(ptr %0)
  %2 = call ptr @tspp_int_to_string(i32 %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %2)
  ret i32 0
}

define i32 @Shape.area(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret i32 0
}

define void @__ctor_Circle(ptr %this, i32 %r) {
entry:
  %r2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %r, ptr %r2, align 4
  %0 = load i32, ptr %r2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Circle, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  ret void
}

define i32 @Circle.area(ptr %this) #1 {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %Circle, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %sitofp = sitofp i32 %1 to float
  %fmultmp = fmul float 0x40091EB860000000, %sitofp
  %2 = load ptr, ptr %this1, align 8
  %field.ptr2 = getelementptr %Circle, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %field.ptr2, align 4
  %sitofp3 = sitofp i32 %3 to float
  %fmultmp4 = fmul float %fmultmp, %sitofp3
  ret float %fmultmp4
}

define void @__ctor_SBox(ptr %this, i32 %v) {
entry:
  %v2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %v, ptr %v2, align 4
  %0 = load i32, ptr %v2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %SBox, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  ret void
}

define i32 @SBox.get(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %SBox, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  ret i32 %1
}

define i32 @SBox.set(ptr %this, i32 %v) {
entry:
  %v2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %v, ptr %v2, align 4
  %0 = load i32, ptr %v2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %SBox, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  ret i32 0
}

define void @__ctor_PAdapter(ptr %this, i32 %x, i32 %y) {
entry:
  %y3 = alloca i32, align 4
  %x2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %x, ptr %x2, align 4
  store i32 %y, ptr %y3, align 4
  %0 = load i32, ptr %x2, align 4
  %1 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %PAdapter, ptr %1, i32 0, i32 0
  store i32 %0, ptr %field.ptr, align 4
  %2 = load i32, ptr %y3, align 4
  %3 = load ptr, ptr %this1, align 8
  %field.ptr4 = getelementptr %PAdapter, ptr %3, i32 0, i32 1
  store i32 %2, ptr %field.ptr4, align 4
  ret void
}

define i32 @PAdapter.toString(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %PAdapter, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr @9, ptr %2)
  %4 = call ptr @tspp_string_concat(ptr %3, ptr @10)
  %5 = load ptr, ptr %this1, align 8
  %field.ptr2 = getelementptr %PAdapter, ptr %5, i32 0, i32 1
  %6 = load i32, ptr %field.ptr2, align 4
  %7 = call ptr @tspp_int_to_string(i32 %6)
  %8 = call ptr @tspp_string_concat(ptr %4, ptr %7)
  %9 = call ptr @tspp_string_concat(ptr %8, ptr @11)
  ret ptr %9
}

define i32 @PAdapter.move(ptr %this, i32 %dx, i32 %dy) {
entry:
  %dy3 = alloca i32, align 4
  %dx2 = alloca i32, align 4
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %dx, ptr %dx2, align 4
  store i32 %dy, ptr %dy3, align 4
  %0 = load ptr, ptr %this1, align 8
  %field.ptr = getelementptr %PAdapter, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %field.ptr, align 4
  %2 = load i32, ptr %dx2, align 4
  %addtmp = add i32 %1, %2
  %3 = load ptr, ptr %this1, align 8
  %field.ptr4 = getelementptr %PAdapter, ptr %3, i32 0, i32 0
  store i32 %addtmp, ptr %field.ptr4, align 4
  %4 = load ptr, ptr %this1, align 8
  %field.ptr5 = getelementptr %PAdapter, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %field.ptr5, align 4
  %6 = load i32, ptr %dy3, align 4
  %addtmp6 = add i32 %5, %6
  %7 = load ptr, ptr %this1, align 8
  %field.ptr7 = getelementptr %PAdapter, ptr %7, i32 0, i32 1
  store i32 %addtmp6, ptr %field.ptr7, align 4
  ret i32 0
}

define i32 @PAdapter.print(ptr %this) {
entry:
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %0 = load ptr, ptr %this1, align 8
  %1 = call i32 @PAdapter.toString(ptr %0)
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call ptr @tspp_string_concat(ptr @12, ptr %2)
  call void @tspp_console_log(ptr %3)
  ret i32 0
}

define i32 @firstOf(i32 %a, i32 %b) {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %0 = load i32, ptr %a1, align 4
  ret i32 %0
}

define i32 @idNum(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  ret i32 %0
}

define i32 @stringifyPoint(i32 %pt) {
entry:
  %pt1 = alloca i32, align 4
  store i32 %pt, ptr %pt1, align 4
  %0 = load i32, ptr %pt1, align 4
  %1 = inttoptr i32 %0 to ptr
  %2 = call i32 @PAdapter.toString(ptr %1)
  %3 = call ptr @tspp_int_to_string(i32 %2)
  %4 = call ptr @tspp_string_concat(ptr @13, ptr %3)
  ret ptr %4
}

define i32 @useBoxString(i32 %bx) {
entry:
  %bx1 = alloca i32, align 4
  store i32 %bx, ptr %bx1, align 4
  %0 = load i32, ptr %bx1, align 4
  %1 = load i32, ptr %bx1, align 4
  %2 = inttoptr i32 %1 to ptr
  %3 = call i32 @SBox.get(ptr %2)
  %4 = call ptr @tspp_int_to_string(i32 %3)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @14)
  %6 = inttoptr i32 %0 to ptr
  %7 = ptrtoint ptr %5 to i64
  %8 = trunc i64 %7 to i32
  %9 = call i32 @SBox.set(ptr %6, i32 %8)
  %10 = load i32, ptr %bx1, align 4
  %11 = inttoptr i32 %10 to ptr
  %12 = call i32 @SBox.get(ptr %11)
  ret i32 %12
}

define i32 @isEven(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  %modtmp = srem i32 %0, 2
  %1 = sext i32 %modtmp to i64
  %icmpeq = icmp eq i64 %1, 0
  ret i1 %icmpeq
}

define i32 @sumTo(i32 %n) {
entry:
  %i = alloca i32, align 4
  %s = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store i32 0, ptr %s, align 4
  store i32 1, ptr %i, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %n1, align 4
  %2 = sext i32 %0 to i64
  %3 = sext i32 %1 to i64
  %icmpole = icmp sle i64 %2, %3
  br i1 %icmpole, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %4 = load i32, ptr %s, align 4
  %5 = load i32, ptr %i, align 4
  %addtmp = add i32 %4, %5
  store i32 %addtmp, ptr %s, align 4
  %6 = load i32, ptr %i, align 4
  %addtmp2 = add i32 %6, 1
  store i32 %addtmp2, ptr %i, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %7 = load i32, ptr %s, align 4
  ret i32 %7
}

define i32 @forVariants() {
entry:
  %k = alloca i32, align 4
  %j = alloca i32, align 4
  %i = alloca i32, align 4
  %total = alloca i32, align 4
  store i32 0, ptr %total, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %1 = sext i32 %0 to i64
  %icmpolt = icmp slt i64 %1, 3
  br i1 %icmpolt, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr %total, align 4
  %3 = load i32, ptr %i, align 4
  %addtmp = add i32 %2, %3
  store i32 %addtmp, ptr %total, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, ptr %i, align 4
  %addtmp1 = add i32 %4, 1
  store i32 %addtmp1, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, ptr %j, align 4
  store i32 0, ptr %j, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc4, %for.end
  %5 = load i32, ptr %j, align 4
  %6 = sext i32 %5 to i64
  %icmpolt6 = icmp slt i64 %6, 2
  br i1 %icmpolt6, label %for.body3, label %for.end5

for.body3:                                        ; preds = %for.cond2
  %7 = load i32, ptr %total, align 4
  %addtmp7 = add i32 %7, 10
  store i32 %addtmp7, ptr %total, align 4
  br label %for.inc4

for.inc4:                                         ; preds = %for.body3
  %8 = load i32, ptr %j, align 4
  %addtmp8 = add i32 %8, 1
  store i32 %addtmp8, ptr %j, align 4
  br label %for.cond2

for.end5:                                         ; preds = %for.cond2
  store i32 0, ptr %k, align 4
  br label %for.cond9

for.cond9:                                        ; preds = %for.inc11, %for.end5
  %9 = load i32, ptr %k, align 4
  %10 = sext i32 %9 to i64
  %icmpolt13 = icmp slt i64 %10, 2
  br i1 %icmpolt13, label %for.body10, label %for.end12

for.body10:                                       ; preds = %for.cond9
  %11 = load i32, ptr %total, align 4
  %addtmp14 = add i32 %11, 100
  store i32 %addtmp14, ptr %total, align 4
  br label %for.inc11

for.inc11:                                        ; preds = %for.body10
  %12 = load i32, ptr %k, align 4
  %addtmp15 = add i32 %12, 1
  store i32 %addtmp15, ptr %k, align 4
  br label %for.cond9

for.end12:                                        ; preds = %for.cond9
  %13 = load i32, ptr %total, align 4
  ret i32 %13
}

define i32 @precedenceDemo() {
entry:
  %bonus = alloca i32, align 4
  %logic = alloca i1, align 1
  %ge = alloca i1, align 1
  %neq = alloca i1, align 1
  %not0 = alloca i32, align 4
  %bxor = alloca i32, align 4
  %bor = alloca i32, align 4
  %band = alloca i32, align 4
  %shiftL = alloca i32, align 4
  %addMul = alloca i32, align 4
  %add = alloca i32, align 4
  %mul = alloca i32, align 4
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 5, ptr %a, align 4
  store i32 3, ptr %b, align 4
  store i32 6, ptr %mul, align 4
  %0 = load i32, ptr %mul, align 4
  %addtmp = add i32 %0, 4
  store i32 %addtmp, ptr %add, align 4
  store i32 14, ptr %addMul, align 4
  store i32 16, ptr %shiftL, align 4
  %1 = load i32, ptr %a, align 4
  %2 = load i32, ptr %b, align 4
  %andtmp = and i32 %1, %2
  store i32 %andtmp, ptr %band, align 4
  %3 = load i32, ptr %a, align 4
  %4 = load i32, ptr %b, align 4
  %ortmp = or i32 %3, %4
  store i32 %ortmp, ptr %bor, align 4
  %5 = load i32, ptr %a, align 4
  %6 = load i32, ptr %b, align 4
  %xortmp = xor i32 %5, %6
  store i32 %xortmp, ptr %bxor, align 4
  store i32 -1, ptr %not0, align 4
  %7 = load i32, ptr %addMul, align 4
  %8 = sext i32 %7 to i64
  %icmpne = icmp ne i64 %8, 13
  store i1 %icmpne, ptr %neq, align 1
  %9 = load i32, ptr %shiftL, align 4
  %10 = sext i32 %9 to i64
  %icmpoge = icmp sge i64 %10, 16
  store i1 %icmpoge, ptr %ge, align 1
  %11 = load i1, ptr %neq, align 1
  br i1 %11, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %12 = load i1, ptr %ge, align 1
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %12, %land.rhs ]
  br i1 %land, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %land.end
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.end
  %lor = phi i1 [ true, %land.end ], [ false, %lor.rhs ]
  store i1 %lor, ptr %logic, align 1
  store i32 0, ptr %bonus, align 4
  %13 = load i1, ptr %logic, align 1
  br i1 %13, label %then, label %ifcont

then:                                             ; preds = %lor.end
  store i32 1, ptr %bonus, align 4
  br label %ifcont

ifcont:                                           ; preds = %then, %lor.end
  %14 = load i32, ptr %addMul, align 4
  %15 = load i32, ptr %shiftL, align 4
  %addtmp1 = add i32 %14, %15
  %16 = load i32, ptr %band, align 4
  %addtmp2 = add i32 %addtmp1, %16
  %17 = load i32, ptr %bor, align 4
  %addtmp3 = add i32 %addtmp2, %17
  %18 = load i32, ptr %bxor, align 4
  %addtmp4 = add i32 %addtmp3, %18
  %19 = load i32, ptr %not0, align 4
  %addtmp5 = add i32 %addtmp4, %19
  %20 = load i32, ptr %bonus, align 4
  %addtmp6 = add i32 %addtmp5, %20
  ret i32 %addtmp6
}

define i32 @memberAndIndexAccess() {
entry:
  %chk = alloca i32, align 4
  %arr1 = alloca ptr, align 8
  %p = alloca ptr, align 8
  %obj = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Point, ptr null, i32 1) to i64))
  call void @__ctor_Point(ptr %obj, i32 1, i32 2)
  store ptr %obj, ptr %p, align 8
  %0 = load ptr, ptr %p, align 8
  %1 = call i32 @PAdapter.move(ptr %0, i32 1, i32 3)
  %arr = call ptr @GC_malloc(i64 12)
  %2 = getelementptr i32, ptr %arr, i32 0
  store i32 10, ptr %2, align 4
  %3 = getelementptr i32, ptr %arr, i32 1
  store i32 20, ptr %3, align 4
  %4 = getelementptr i32, ptr %arr, i32 2
  store i32 30, ptr %4, align 4
  store ptr %arr, ptr %arr1, align 8
  %5 = load ptr, ptr %arr1, align 8
  %elem.ptr = getelementptr i32, ptr %5, i32 1
  %6 = load i32, ptr %elem.ptr, align 4
  %addtmp = add i32 %6, 79
  %7 = load ptr, ptr %arr1, align 8
  %elem.ptr2 = getelementptr i32, ptr %7, i32 1
  store i32 %addtmp, ptr %elem.ptr2, align 4
  %8 = load ptr, ptr %arr1, align 8
  %elem.ptr3 = getelementptr i32, ptr %8, i32 0
  %9 = load i32, ptr %elem.ptr3, align 4
  %10 = load ptr, ptr %arr1, align 8
  %elem.ptr4 = getelementptr i32, ptr %10, i32 1
  %11 = load i32, ptr %elem.ptr4, align 4
  %addtmp5 = add i32 %9, %11
  %12 = load ptr, ptr %arr1, align 8
  %elem.ptr6 = getelementptr i32, ptr %12, i32 2
  %13 = load i32, ptr %elem.ptr6, align 4
  %addtmp7 = add i32 %addtmp5, %13
  store i32 %addtmp7, ptr %chk, align 4
  %14 = load i32, ptr %chk, align 4
  %addtmp8 = add i32 %14, 2
  %addtmp9 = add i32 %addtmp8, 5
  ret i32 %addtmp9
}

define i32 @pointerPlay() {
entry:
  %ok = alloca i32, align 4
  %sp3 = alloca i32, align 4
  %sp2 = alloca i32, align 4
  %sp1 = alloca i32, align 4
  %pf = alloca ptr, align 8
  %pi = alloca ptr, align 8
  store ptr null, ptr %pi, align 8
  store ptr null, ptr %pf, align 8
  store i32 0, ptr %ok, align 4
  %0 = load ptr, ptr %pi, align 8
  %1 = ptrtoint ptr %0 to i64
  %icmpeq = icmp eq i64 %1, 0
  br i1 %icmpeq, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %2 = load ptr, ptr %pf, align 8
  %3 = ptrtoint ptr %2 to i64
  %icmpeq1 = icmp eq i64 %3, 0
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %icmpeq1, %land.rhs ]
  br i1 %land, label %then, label %ifcont

then:                                             ; preds = %land.end
  %4 = load i32, ptr %ok, align 4
  %addtmp = add i32 %4, 1
  store i32 %addtmp, ptr %ok, align 4
  br label %ifcont

ifcont:                                           ; preds = %then, %land.end
  %5 = load i32, ptr %sp1, align 4
  %6 = sext i32 %5 to i64
  %icmpeq2 = icmp eq i64 %6, 0
  br i1 %icmpeq2, label %land.rhs3, label %land.end4

land.rhs3:                                        ; preds = %ifcont
  %7 = load i32, ptr %sp2, align 4
  %8 = sext i32 %7 to i64
  %icmpeq5 = icmp eq i64 %8, 0
  br label %land.end4

land.end4:                                        ; preds = %land.rhs3, %ifcont
  %land6 = phi i1 [ false, %ifcont ], [ %icmpeq5, %land.rhs3 ]
  br i1 %land6, label %land.rhs7, label %land.end8

land.rhs7:                                        ; preds = %land.end4
  %9 = load i32, ptr %sp3, align 4
  %10 = sext i32 %9 to i64
  %icmpeq9 = icmp eq i64 %10, 0
  br label %land.end8

land.end8:                                        ; preds = %land.rhs7, %land.end4
  %land10 = phi i1 [ false, %land.end4 ], [ %icmpeq9, %land.rhs7 ]
  br i1 %land10, label %then11, label %ifcont12

then11:                                           ; preds = %land.end8
  %11 = load i32, ptr %ok, align 4
  %addtmp13 = add i32 %11, 1
  store i32 %addtmp13, ptr %ok, align 4
  br label %ifcont12

ifcont12:                                         ; preds = %then11, %land.end8
  %12 = load i32, ptr %ok, align 4
  ret i32 %12
}

define i32 @describePointLike(i32 %p) {
entry:
  %p1 = alloca i32, align 4
  store i32 %p, ptr %p1, align 4
  %0 = load i32, ptr %p1, align 4
  %1 = inttoptr i32 %0 to ptr
  %2 = call i32 @Dog.print(ptr %1)
  %3 = load i32, ptr %p1, align 4
  %4 = inttoptr i32 %3 to ptr
  %5 = call i32 @PAdapter.toString(ptr %4)
  %6 = call ptr @tspp_int_to_string(i32 %5)
  %7 = call ptr @tspp_string_concat(ptr @15, ptr %6)
  ret ptr %7
}

define i32 @storagePlay() {
entry:
  %sy = alloca i32, align 4
  %hx = alloca i32, align 4
  store i32 10, ptr %hx, align 4
  store i32 32, ptr %sy, align 4
  %0 = load i32, ptr %hx, align 4
  %1 = load i32, ptr %sy, align 4
  %addtmp = add i32 %0, %1
  ret i32 %addtmp
}

define i32 @main() {
entry:
  %L = alloca i1, align 1
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  %n = alloca i32, align 4
  %pl = alloca ptr, align 8
  %bxI = alloca ptr, align 8
  %bxS = alloca ptr, align 8
  %rh = alloca ptr, align 8
  %ba = alloca ptr, align 8
  %c = alloca ptr, align 8
  %d = alloca ptr, align 8
  %a = alloca ptr, align 8
  %sb = alloca ptr, align 8
  %p1 = alloca ptr, align 8
  %flexible = alloca ptr, align 8
  %ident = alloca i32, align 4
  %dist = alloca float, align 4
  store float 3.500000e+00, ptr %dist, align 4
  store i32 42, ptr %ident, align 4
  store ptr @16, ptr %flexible, align 8
  %0 = load float, ptr %dist, align 4
  %1 = call ptr @tspp_float_to_string(float %0)
  %2 = call ptr @tspp_string_concat(ptr @17, ptr %1)
  call void @tspp_console_log(ptr %2)
  call void @tspp_free_string(ptr %1)
  %3 = load i32, ptr %ident, align 4
  %4 = call ptr @tspp_int_to_string(i32 %3)
  %5 = call ptr @tspp_string_concat(ptr @18, ptr %4)
  call void @tspp_console_log(ptr %5)
  call void @tspp_free_string(ptr %4)
  %6 = load ptr, ptr %flexible, align 8
  %7 = call ptr @tspp_string_concat(ptr @19, ptr %6)
  call void @tspp_console_log(ptr %7)
  %obj = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Point, ptr null, i32 1) to i64))
  call void @__ctor_Point(ptr %obj, i32 1, i32 2)
  store ptr %obj, ptr %p1, align 8
  %8 = load ptr, ptr %p1, align 8
  %9 = call i32 @PAdapter.toString(ptr %8)
  %10 = call ptr @tspp_int_to_string(i32 %9)
  %11 = call ptr @tspp_string_concat(ptr @20, ptr %10)
  call void @tspp_console_log(ptr %11)
  call void @tspp_free_string(ptr %10)
  %12 = load ptr, ptr %p1, align 8
  %13 = call i32 @PAdapter.move(ptr %12, i32 0, i32 0)
  %14 = load ptr, ptr %p1, align 8
  %15 = call i32 @PAdapter.toString(ptr %14)
  %16 = call ptr @tspp_int_to_string(i32 %15)
  %17 = call ptr @tspp_string_concat(ptr @21, ptr %16)
  call void @tspp_console_log(ptr %17)
  call void @tspp_free_string(ptr %16)
  %18 = call i32 @Point.stats()
  %19 = call ptr @tspp_int_to_string(i32 %18)
  call void @tspp_console_log(ptr %19)
  call void @tspp_free_string(ptr %19)
  %20 = load ptr, ptr %p1, align 8
  %21 = ptrtoint ptr %20 to i64
  %22 = trunc i64 %21 to i32
  %23 = call i32 @stringifyPoint(i32 %22)
  %24 = call ptr @tspp_int_to_string(i32 %23)
  %25 = call ptr @tspp_string_concat(ptr @22, ptr %24)
  call void @tspp_console_log(ptr %25)
  call void @tspp_free_string(ptr %24)
  %obj1 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%SBox, ptr null, i32 1) to i64))
  call void @__ctor_SBox(ptr %obj1, ptr @23)
  store ptr %obj1, ptr %sb, align 8
  %26 = load ptr, ptr %sb, align 8
  %27 = ptrtoint ptr %26 to i64
  %28 = trunc i64 %27 to i32
  %29 = call i32 @useBoxString(i32 %28)
  %30 = call ptr @tspp_int_to_string(i32 %29)
  %31 = call ptr @tspp_string_concat(ptr @24, ptr %30)
  call void @tspp_console_log(ptr %31)
  call void @tspp_free_string(ptr %30)
  %obj2 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Animal, ptr null, i32 1) to i64))
  call void @__ctor_Animal(ptr %obj2, ptr @25)
  store ptr %obj2, ptr %a, align 8
  %32 = load ptr, ptr %a, align 8
  %33 = call i32 @Dog.speak(ptr %32)
  %34 = call ptr @tspp_int_to_string(i32 %33)
  %35 = call ptr @tspp_string_concat(ptr @26, ptr %34)
  call void @tspp_console_log(ptr %35)
  call void @tspp_free_string(ptr %34)
  %36 = call i32 @Animal.kingdom()
  %37 = call ptr @tspp_int_to_string(i32 %36)
  %38 = call ptr @tspp_string_concat(ptr @27, ptr %37)
  call void @tspp_console_log(ptr %38)
  call void @tspp_free_string(ptr %37)
  %obj3 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Dog, ptr null, i32 1) to i64))
  call void @__ctor_Dog(ptr %obj3, ptr @28, ptr @29)
  store ptr %obj3, ptr %d, align 8
  %39 = load ptr, ptr %d, align 8
  %40 = call i32 @Dog.speak(ptr %39)
  %41 = call ptr @tspp_int_to_string(i32 %40)
  %42 = call ptr @tspp_string_concat(ptr @30, ptr %41)
  call void @tspp_console_log(ptr %42)
  call void @tspp_free_string(ptr %41)
  %43 = load ptr, ptr %d, align 8
  %44 = call i32 @Dog.getBreed(ptr %43)
  %45 = call ptr @tspp_int_to_string(i32 %44)
  %46 = call ptr @tspp_string_concat(ptr @31, ptr %45)
  call void @tspp_console_log(ptr %46)
  call void @tspp_free_string(ptr %45)
  %47 = load ptr, ptr %d, align 8
  %48 = call i32 @Dog.print(ptr %47)
  %obj4 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Circle, ptr null, i32 1) to i64))
  call void @__ctor_Circle(ptr %obj4, i32 2)
  store ptr %obj4, ptr %c, align 8
  %49 = load ptr, ptr %c, align 8
  %50 = call i32 @Circle.area(ptr %49)
  %51 = call ptr @tspp_int_to_string(i32 %50)
  %52 = call ptr @tspp_string_concat(ptr @32, ptr %51)
  call void @tspp_console_log(ptr %52)
  call void @tspp_free_string(ptr %51)
  %obj5 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%ByteArray, ptr null, i32 1) to i64))
  call void @__ctor_ByteArray(ptr %obj5, i32 3)
  store ptr %obj5, ptr %ba, align 8
  %53 = load ptr, ptr %ba, align 8
  %54 = call i32 @SBox.set(ptr %53, i32 0, i32 42)
  %55 = load ptr, ptr %ba, align 8
  %56 = call i32 @SBox.get(ptr %55, i32 0)
  %57 = call ptr @tspp_int_to_string(i32 %56)
  %58 = call ptr @tspp_string_concat(ptr @33, ptr %57)
  call void @tspp_console_log(ptr %58)
  call void @tspp_free_string(ptr %57)
  %obj6 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%ResourceHandle, ptr null, i32 1) to i64))
  call void @__ctor_ResourceHandle(ptr %obj6, i32 7)
  store ptr %obj6, ptr %rh, align 8
  %59 = load ptr, ptr %rh, align 8
  %60 = call i32 @Dog.print(ptr %59)
  %61 = call ptr @tspp_ptr_to_string(ptr @35)
  %62 = call ptr @tspp_int_to_string(i32 20)
  %63 = call ptr @tspp_string_concat(ptr @34, ptr %61)
  %64 = call ptr @tspp_string_concat(ptr %63, ptr %62)
  call void @tspp_console_log(ptr %64)
  call void @tspp_free_string(ptr %61)
  call void @tspp_free_string(ptr %62)
  %obj7 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%SBox, ptr null, i32 1) to i64))
  call void @__ctor_SBox(ptr %obj7, ptr @36)
  store ptr %obj7, ptr %bxS, align 8
  %obj8 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%SBox, ptr null, i32 1) to i64))
  call void @__ctor_SBox(ptr %obj8, i32 5)
  store ptr %obj8, ptr %bxI, align 8
  %65 = load ptr, ptr %bxS, align 8
  %66 = call i32 @SBox.get(ptr %65)
  %67 = call ptr @tspp_int_to_string(i32 %66)
  %68 = call ptr @tspp_string_concat(ptr @37, ptr %67)
  call void @tspp_console_log(ptr %68)
  call void @tspp_free_string(ptr %67)
  %69 = load ptr, ptr %bxI, align 8
  %70 = call i32 @SBox.get(ptr %69)
  %71 = call ptr @tspp_int_to_string(i32 %70)
  %72 = call ptr @tspp_string_concat(ptr @38, ptr %71)
  call void @tspp_console_log(ptr %72)
  call void @tspp_free_string(ptr %71)
  %obj9 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%PAdapter, ptr null, i32 1) to i64))
  call void @__ctor_PAdapter(ptr %obj9, i32 2, i32 3)
  store ptr %obj9, ptr %pl, align 8
  %73 = load ptr, ptr %pl, align 8
  %74 = ptrtoint ptr %73 to i64
  %75 = trunc i64 %74 to i32
  %76 = call i32 @describePointLike(i32 %75)
  %77 = call ptr @tspp_int_to_string(i32 %76)
  call void @tspp_console_log(ptr %77)
  call void @tspp_free_string(ptr %77)
  store i32 6, ptr %n, align 4
  %78 = load i32, ptr %n, align 4
  %79 = call i32 @isEven(i32 %78)
  %80 = call ptr @tspp_int_to_string(i32 %79)
  %81 = call ptr @tspp_string_concat(ptr @39, ptr %80)
  call void @tspp_console_log(ptr %81)
  call void @tspp_free_string(ptr %80)
  %82 = call i32 @sumTo(i32 5)
  %83 = call ptr @tspp_int_to_string(i32 %82)
  %84 = call ptr @tspp_string_concat(ptr @40, ptr %83)
  call void @tspp_console_log(ptr %84)
  call void @tspp_free_string(ptr %83)
  %85 = call i32 @forVariants()
  %86 = call ptr @tspp_int_to_string(i32 %85)
  %87 = call ptr @tspp_string_concat(ptr @41, ptr %86)
  call void @tspp_console_log(ptr %87)
  call void @tspp_free_string(ptr %86)
  %88 = call i32 @precedenceDemo()
  %89 = call ptr @tspp_int_to_string(i32 %88)
  %90 = call ptr @tspp_string_concat(ptr @42, ptr %89)
  call void @tspp_console_log(ptr %90)
  call void @tspp_free_string(ptr %89)
  %91 = call i32 @memberAndIndexAccess()
  %92 = call ptr @tspp_int_to_string(i32 %91)
  %93 = call ptr @tspp_string_concat(ptr @43, ptr %92)
  call void @tspp_console_log(ptr %93)
  call void @tspp_free_string(ptr %92)
  store i32 0, ptr %x, align 4
  store i32 0, ptr %y, align 4
  store i32 5, ptr %y, align 4
  store i32 5, ptr %x, align 4
  %94 = load i32, ptr %x, align 4
  %95 = call ptr @tspp_int_to_string(i32 %94)
  %96 = call ptr @tspp_string_concat(ptr @44, ptr %95)
  call void @tspp_console_log(ptr %96)
  call void @tspp_free_string(ptr %95)
  %97 = load i32, ptr %y, align 4
  %98 = call ptr @tspp_int_to_string(i32 %97)
  %99 = call ptr @tspp_string_concat(ptr @45, ptr %98)
  call void @tspp_console_log(ptr %99)
  call void @tspp_free_string(ptr %98)
  br i1 false, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ poison, %land.rhs ]
  br i1 %land, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %land.end
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %land.end
  %lor = phi i1 [ true, %land.end ], [ true, %lor.rhs ]
  store i1 %lor, ptr %L, align 1
  %100 = load i1, ptr %L, align 1
  %101 = call ptr @tspp_bool_to_string(i1 %100)
  %102 = call ptr @tspp_string_concat(ptr @46, ptr %101)
  call void @tspp_console_log(ptr %102)
  call void @tspp_free_string(ptr %101)
  %103 = call ptr @tspp_int_to_string(i32 8)
  %104 = call ptr @tspp_string_concat(ptr @47, ptr %103)
  call void @tspp_console_log(ptr %104)
  call void @tspp_free_string(ptr %103)
  %105 = call ptr @tspp_int_to_string(i32 4)
  %106 = call ptr @tspp_string_concat(ptr @48, ptr %105)
  call void @tspp_console_log(ptr %106)
  call void @tspp_free_string(ptr %105)
  %107 = call ptr @tspp_int_to_string(i32 -1)
  %108 = call ptr @tspp_string_concat(ptr @49, ptr %107)
  call void @tspp_console_log(ptr %108)
  call void @tspp_free_string(ptr %107)
  %109 = call i32 @pointerPlay()
  %110 = call ptr @tspp_int_to_string(i32 %109)
  %111 = call ptr @tspp_string_concat(ptr @50, ptr %110)
  call void @tspp_console_log(ptr %111)
  call void @tspp_free_string(ptr %110)
  %112 = call i32 @storagePlay()
  %113 = call ptr @tspp_int_to_string(i32 %112)
  %114 = call ptr @tspp_string_concat(ptr @51, ptr %113)
  call void @tspp_console_log(ptr %114)
  call void @tspp_free_string(ptr %113)
  ret i32 0
}

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_ptr_to_string(ptr)

declare ptr @tspp_bool_to_string(i1)

attributes #0 = { "tspp.static" }
attributes #1 = { "tspp.override" }
