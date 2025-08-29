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
  %3 = ptrtoint ptr %2 to i64
  %4 = trunc i64 %3 to i32
  ret i32 %4
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
  %10 = ptrtoint ptr %9 to i64
  %11 = trunc i64 %10 to i32
  ret i32 %11
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
  %4 = ptrtoint ptr %3 to i64
  %5 = trunc i64 %4 to i32
  ret i32 %5
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
  %9 = ptrtoint ptr %8 to i64
  %10 = trunc i64 %9 to i32
  ret i32 %10
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
  %4 = fptosi float %fmultmp4 to i32
  ret i32 %4
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
  %10 = ptrtoint ptr %9 to i64
  %11 = trunc i64 %10 to i32
  ret i32 %11
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
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call i32 @PAdapter.toString(ptr %1)
  %3 = call ptr @tspp_int_to_string(i32 %2)
  %4 = call ptr @tspp_string_concat(ptr @13, ptr %3)
  %5 = ptrtoint ptr %4 to i64
  %6 = trunc i64 %5 to i32
  ret i32 %6
}

define i32 @useBoxString(i32 %bx) {
entry:
  %bx1 = alloca i32, align 4
  store i32 %bx, ptr %bx1, align 4
  %0 = load i32, ptr %bx1, align 4
  %1 = load i32, ptr %bx1, align 4
  %2 = call ptr @tspp_int_to_string(i32 %1)
  %3 = call i32 @SBox.get(ptr %2)
  %4 = call ptr @tspp_int_to_string(i32 %3)
  %5 = call ptr @tspp_string_concat(ptr %4, ptr @14)
  %6 = call ptr @tspp_int_to_string(i32 %0)
  %7 = call i32 @SBox.set(ptr %6, ptr %5)
  %8 = load i32, ptr %bx1, align 4
  %9 = call ptr @tspp_int_to_string(i32 %8)
  %10 = call i32 @SBox.get(ptr %9)
  ret i32 %10
}

define i32 @isEven(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %0 = load i32, ptr %n1, align 4
  %modtmp = srem i32 %0, 2
  %1 = sext i32 %modtmp to i64
  %icmpeq = icmp eq i64 %1, 0
  %2 = sext i1 %icmpeq to i32
  ret i32 %2
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
  %1 = call ptr @tspp_int_to_string(i32 %0)
  %2 = call i32 @Dog.print(ptr %1)
  %3 = load i32, ptr %p1, align 4
  %4 = call ptr @tspp_int_to_string(i32 %3)
  %5 = call i32 @PAdapter.toString(ptr %4)
  %6 = call ptr @tspp_int_to_string(i32 %5)
  %7 = call ptr @tspp_string_concat(ptr @15, ptr %6)
  %8 = ptrtoint ptr %7 to i64
  %9 = trunc i64 %8 to i32
  ret i32 %9
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
  %21 = call i32 @stringifyPoint(ptr %20)
  %22 = call ptr @tspp_int_to_string(i32 %21)
  %23 = call ptr @tspp_string_concat(ptr @22, ptr %22)
  call void @tspp_console_log(ptr %23)
  call void @tspp_free_string(ptr %22)
  %obj1 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%SBox, ptr null, i32 1) to i64))
  call void @__ctor_SBox(ptr %obj1, ptr @23)
  store ptr %obj1, ptr %sb, align 8
  %24 = load ptr, ptr %sb, align 8
  %25 = call i32 @useBoxString(ptr %24)
  %26 = call ptr @tspp_int_to_string(i32 %25)
  %27 = call ptr @tspp_string_concat(ptr @24, ptr %26)
  call void @tspp_console_log(ptr %27)
  call void @tspp_free_string(ptr %26)
  %obj2 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Animal, ptr null, i32 1) to i64))
  call void @__ctor_Animal(ptr %obj2, ptr @25)
  store ptr %obj2, ptr %a, align 8
  %28 = load ptr, ptr %a, align 8
  %29 = call i32 @Dog.speak(ptr %28)
  %30 = call ptr @tspp_int_to_string(i32 %29)
  %31 = call ptr @tspp_string_concat(ptr @26, ptr %30)
  call void @tspp_console_log(ptr %31)
  call void @tspp_free_string(ptr %30)
  %32 = call i32 @Animal.kingdom()
  %33 = call ptr @tspp_int_to_string(i32 %32)
  %34 = call ptr @tspp_string_concat(ptr @27, ptr %33)
  call void @tspp_console_log(ptr %34)
  call void @tspp_free_string(ptr %33)
  %obj3 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Dog, ptr null, i32 1) to i64))
  call void @__ctor_Dog(ptr %obj3, ptr @28, ptr @29)
  store ptr %obj3, ptr %d, align 8
  %35 = load ptr, ptr %d, align 8
  %36 = call i32 @Dog.speak(ptr %35)
  %37 = call ptr @tspp_int_to_string(i32 %36)
  %38 = call ptr @tspp_string_concat(ptr @30, ptr %37)
  call void @tspp_console_log(ptr %38)
  call void @tspp_free_string(ptr %37)
  %39 = load ptr, ptr %d, align 8
  %40 = call i32 @Dog.getBreed(ptr %39)
  %41 = call ptr @tspp_int_to_string(i32 %40)
  %42 = call ptr @tspp_string_concat(ptr @31, ptr %41)
  call void @tspp_console_log(ptr %42)
  call void @tspp_free_string(ptr %41)
  %43 = load ptr, ptr %d, align 8
  %44 = call i32 @Dog.print(ptr %43)
  %obj4 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%Circle, ptr null, i32 1) to i64))
  call void @__ctor_Circle(ptr %obj4, i32 2)
  store ptr %obj4, ptr %c, align 8
  %45 = load ptr, ptr %c, align 8
  %46 = call i32 @Circle.area(ptr %45)
  %47 = call ptr @tspp_int_to_string(i32 %46)
  %48 = call ptr @tspp_string_concat(ptr @32, ptr %47)
  call void @tspp_console_log(ptr %48)
  call void @tspp_free_string(ptr %47)
  %obj5 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%ByteArray, ptr null, i32 1) to i64))
  call void @__ctor_ByteArray(ptr %obj5, i32 3)
  store ptr %obj5, ptr %ba, align 8
  %49 = load ptr, ptr %ba, align 8
  %50 = call i32 @SBox.set(ptr %49, i32 0, i32 42)
  %51 = load ptr, ptr %ba, align 8
  %52 = call i32 @SBox.get(ptr %51, i32 0)
  %53 = call ptr @tspp_int_to_string(i32 %52)
  %54 = call ptr @tspp_string_concat(ptr @33, ptr %53)
  call void @tspp_console_log(ptr %54)
  call void @tspp_free_string(ptr %53)
  %obj6 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%ResourceHandle, ptr null, i32 1) to i64))
  call void @__ctor_ResourceHandle(ptr %obj6, i32 7)
  store ptr %obj6, ptr %rh, align 8
  %55 = load ptr, ptr %rh, align 8
  %56 = call i32 @Dog.print(ptr %55)
  %57 = call ptr @tspp_ptr_to_string(ptr @35)
  %58 = call ptr @tspp_int_to_string(i32 20)
  %59 = call ptr @tspp_string_concat(ptr @34, ptr %57)
  %60 = call ptr @tspp_string_concat(ptr %59, ptr %58)
  call void @tspp_console_log(ptr %60)
  call void @tspp_free_string(ptr %57)
  call void @tspp_free_string(ptr %58)
  %obj7 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%SBox, ptr null, i32 1) to i64))
  call void @__ctor_SBox(ptr %obj7, ptr @36)
  store ptr %obj7, ptr %bxS, align 8
  %obj8 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%SBox, ptr null, i32 1) to i64))
  call void @__ctor_SBox(ptr %obj8, i32 5)
  store ptr %obj8, ptr %bxI, align 8
  %61 = load ptr, ptr %bxS, align 8
  %62 = call i32 @SBox.get(ptr %61)
  %63 = call ptr @tspp_int_to_string(i32 %62)
  %64 = call ptr @tspp_string_concat(ptr @37, ptr %63)
  call void @tspp_console_log(ptr %64)
  call void @tspp_free_string(ptr %63)
  %65 = load ptr, ptr %bxI, align 8
  %66 = call i32 @SBox.get(ptr %65)
  %67 = call ptr @tspp_int_to_string(i32 %66)
  %68 = call ptr @tspp_string_concat(ptr @38, ptr %67)
  call void @tspp_console_log(ptr %68)
  call void @tspp_free_string(ptr %67)
  %obj9 = call ptr @GC_malloc(i64 ptrtoint (ptr getelementptr (%PAdapter, ptr null, i32 1) to i64))
  call void @__ctor_PAdapter(ptr %obj9, i32 2, i32 3)
  store ptr %obj9, ptr %pl, align 8
  %69 = load ptr, ptr %pl, align 8
  %70 = call i32 @describePointLike(ptr %69)
  %71 = call ptr @tspp_int_to_string(i32 %70)
  call void @tspp_console_log(ptr %71)
  call void @tspp_free_string(ptr %71)
  store i32 6, ptr %n, align 4
  %72 = load i32, ptr %n, align 4
  %73 = call i32 @isEven(i32 %72)
  %74 = call ptr @tspp_int_to_string(i32 %73)
  %75 = call ptr @tspp_string_concat(ptr @39, ptr %74)
  call void @tspp_console_log(ptr %75)
  call void @tspp_free_string(ptr %74)
  %76 = call i32 @sumTo(i32 5)
  %77 = call ptr @tspp_int_to_string(i32 %76)
  %78 = call ptr @tspp_string_concat(ptr @40, ptr %77)
  call void @tspp_console_log(ptr %78)
  call void @tspp_free_string(ptr %77)
  %79 = call i32 @forVariants()
  %80 = call ptr @tspp_int_to_string(i32 %79)
  %81 = call ptr @tspp_string_concat(ptr @41, ptr %80)
  call void @tspp_console_log(ptr %81)
  call void @tspp_free_string(ptr %80)
  %82 = call i32 @precedenceDemo()
  %83 = call ptr @tspp_int_to_string(i32 %82)
  %84 = call ptr @tspp_string_concat(ptr @42, ptr %83)
  call void @tspp_console_log(ptr %84)
  call void @tspp_free_string(ptr %83)
  %85 = call i32 @memberAndIndexAccess()
  %86 = call ptr @tspp_int_to_string(i32 %85)
  %87 = call ptr @tspp_string_concat(ptr @43, ptr %86)
  call void @tspp_console_log(ptr %87)
  call void @tspp_free_string(ptr %86)
  store i32 0, ptr %x, align 4
  store i32 0, ptr %y, align 4
  store i32 5, ptr %y, align 4
  store i32 5, ptr %x, align 4
  %88 = load i32, ptr %x, align 4
  %89 = call ptr @tspp_int_to_string(i32 %88)
  %90 = call ptr @tspp_string_concat(ptr @44, ptr %89)
  call void @tspp_console_log(ptr %90)
  call void @tspp_free_string(ptr %89)
  %91 = load i32, ptr %y, align 4
  %92 = call ptr @tspp_int_to_string(i32 %91)
  %93 = call ptr @tspp_string_concat(ptr @45, ptr %92)
  call void @tspp_console_log(ptr %93)
  call void @tspp_free_string(ptr %92)
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
  %94 = load i1, ptr %L, align 1
  %95 = call ptr @tspp_bool_to_string(i1 %94)
  %96 = call ptr @tspp_string_concat(ptr @46, ptr %95)
  call void @tspp_console_log(ptr %96)
  call void @tspp_free_string(ptr %95)
  %97 = call ptr @tspp_int_to_string(i32 8)
  %98 = call ptr @tspp_string_concat(ptr @47, ptr %97)
  call void @tspp_console_log(ptr %98)
  call void @tspp_free_string(ptr %97)
  %99 = call ptr @tspp_int_to_string(i32 4)
  %100 = call ptr @tspp_string_concat(ptr @48, ptr %99)
  call void @tspp_console_log(ptr %100)
  call void @tspp_free_string(ptr %99)
  %101 = call ptr @tspp_int_to_string(i32 -1)
  %102 = call ptr @tspp_string_concat(ptr @49, ptr %101)
  call void @tspp_console_log(ptr %102)
  call void @tspp_free_string(ptr %101)
  %103 = call i32 @pointerPlay()
  %104 = call ptr @tspp_int_to_string(i32 %103)
  %105 = call ptr @tspp_string_concat(ptr @50, ptr %104)
  call void @tspp_console_log(ptr %105)
  call void @tspp_free_string(ptr %104)
  %106 = call i32 @storagePlay()
  %107 = call ptr @tspp_int_to_string(i32 %106)
  %108 = call ptr @tspp_string_concat(ptr @51, ptr %107)
  call void @tspp_console_log(ptr %108)
  call void @tspp_free_string(ptr %107)
  ret i32 0
}

declare ptr @tspp_float_to_string(float)

declare ptr @tspp_ptr_to_string(ptr)

declare ptr @tspp_bool_to_string(i1)

attributes #0 = { "tspp.static" }
attributes #1 = { "tspp.override" }
