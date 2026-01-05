# tspp Language Tokens and Keywords

## Keywords
```
let const function class interface enum constructor typedef namespace template new get set cast where throws public private protected if else switch case default while do for of break continue return try catch finally throw void int float boolean string stack heap static shared unique weak attribute inline virtual unsafe simd target ref aligned packed abstract zerocast extends implements const_expr sizeof alignof typeof asm identifier number string_literal char_literal true false null_value undefined this plus minus star slash percent ampersand pipe caret tilde right_shift left_shift exclaim ampersand_ampersand pipe_pipe equals equals_equals exclaim_equals less greater less_equals greater_equals plus_equals minus_equals star_equals slash_equals percent_equals ampersand_equals pipe_equals caret_equals plus_plus minus_minus question colon arrow dot at left_paren right_paren left_brace right_brace left_bracket right_bracket semicolon comma yield async await error_token end_of_file
```

## Token Descriptions and Usage
| Token         | Description & Usage |
|---------------|--------------------|
| let           | Declares a mutable variable. <br> `let x = 5;` |
| const         | Declares an immutable constant. <br> `const PI = 3.14;` |
| function      | Declares a function. <br> `function foo(a: int): int {}` |
| class         | Declares a class type. <br> `class MyClass {}` |
| interface     | Declares an interface type. <br> `interface IShape {}` |
| enum          | Declares an enumeration. <br> `enum Color { Red, Green }` |
| constructor   | Class constructor method for initialization. <br> `constructor(x) {}` |
| typedef       | Creates a type alias. <br> `typedef Num = int | float;` |
| namespace     | Declares a namespace for scoping. <br> `namespace Math {}` |
| template      | Declares a generic template. <br> `template<T> ...` |
| new           | Instantiates a new object. <br> `new MyClass()` |
| get           | Declares a property getter. <br> `get value() {}` |
| set           | Declares a property setter. <br> `set value(v) {}` |
| cast          | Performs a type cast. <br> `cast<int>(x)` |
| where         | Specifies generic constraints. <br> `function<T where T:Base>()` |
| throws        | Declares thrown exceptions. <br> `function f() throws Error` |
| public        | Public access modifier. <br> `public x;` |
| private       | Private access modifier. <br> `private x;` |
| protected     | Protected access modifier. <br> `protected x;` |
| if            | Conditional statement. <br> `if (x > 0) { ... }` |
| else          | Alternative branch for `if`. <br> `else { ... }` |
| switch        | Multi-branch selection. <br> `switch (x) { ... }` |
| case          | Case label in switch. <br> `case 1:` |
| default       | Default label in switch. <br> `default:` |
| while         | Loop while condition is true. <br> `while (x < 10) { ... }` |
| do            | Do-while loop. <br> `do { ... } while (x < 10);` |
| for           | For loop. <br> `for (i = 0; i < 10; i++) { ... }` |
| of            | For-of loop for iterables. <br> `for (x of arr) {}` |
| break         | Exits a loop or switch. <br> `break;` |
| continue      | Skips to next loop iteration. <br> `continue;` |
| return        | Returns a value from a function. <br> `return x;` |
| try           | Try block for exception handling. <br> `try { ... }` |
| catch         | Catch block for exceptions. <br> `catch (e) {}` |
| finally       | Finally block after try/catch. <br> `finally {}` |
| throw         | Throws an exception. <br> `throw Error();` |
| void          | Void type (no value). <br> `function f(): void {}` |
| int           | Integer type. <br> `let x: int = 42;` |
| float         | Floating-point type. <br> `let y: float = 3.14;` |
| boolean       | Boolean type. <br> `let b: boolean = true;` |
| string        | String type. <br> `let s: string = "hi";` |
| stack         | Allocates on stack. <br> `#stack int x = 5;` |
| heap          | Allocates on heap. <br> `#heap MyClass obj;` |
| static        | Static storage duration. <br> `#static int counter;` |
| shared        | Shared smart pointer. <br> `#shared<MyClass> obj;` |
| unique        | Unique smart pointer. <br> `#unique<MyClass> obj;` |
| weak          | Weak smart pointer. <br> `#weak<MyClass> obj;` |
| attribute     | Attribute marker. <br> `#inline` |
| inline        | Inline function modifier. <br> `#inline function f() {}` |
| virtual       | Virtual function modifier. <br> `#virtual function f() {}` |
| unsafe        | Marks unsafe code. <br> `#unsafe { ... }` |
| simd          | SIMD vectorization. <br> `#simd float[4] v;` |
| target        | Platform-specific code. <br> `#target("x86")` |
| ref           | Reference parameter. <br> `function f(ref x: int)` |
| aligned       | Memory alignment. <br> `#aligned(64)` |
| packed        | Packed memory layout. <br> `#packed struct S {}` |
| abstract      | Abstract class modifier. <br> `abstract class A {}` |
| zerocast      | Zero-cost abstraction. <br> `#zerocast interface I {}` |
| extends       | Inheritance. <br> `class B extends A {}` |
| implements    | Interface implementation. <br> `class C implements I {}` |
| const_expr    | Compile-time constant. <br> `#const int N = 10;` |
| sizeof        | Sizeof operator. <br> `#sizeof(int)` |
| alignof       | Alignment operator. <br> `#alignof(int)` |
| typeof        | Typeof operator. <br> `#typeof(x)` |
| asm           | Inline assembly. <br> `#asm { ... }` |
| identifier    | Variable/function name. <br> `let x = 5;` |
| number        | Numeric literal. <br> `42`, `3.14` |
| string_literal| String literal. <br> `"hello"` |
| char_literal  | Character literal. <br> `'a'` |
| true/false    | Boolean literal. <br> `true`, `false` |
| null_value    | Null value. <br> `let x = null;` |
| undefined     | Undefined value. <br> `let x = undefined;` |
| this          | Current object reference. <br> `this.x` |
| plus/minus/star/slash/percent | Arithmetic operators. <br> `x + y`, `x - y`, ... |
| ampersand/pipe/caret/tilde/right_shift/left_shift | Bitwise operators. <br> `x & y`, `x | y`, ... |
| exclaim/ampersand_ampersand/pipe_pipe | Logical operators. <br> `!x`, `x && y`, ... |
| equals/equals_equals/exclaim_equals/less/greater/less_equals/greater_equals | Comparison operators. <br> `x == y`, `x != y`, ... |
| plus_equals/minus_equals/star_equals/slash_equals/percent_equals/ampersand_equals/pipe_equals/caret_equals | Compound assignment. <br> `x += 1`, ... |
| plus_plus/minus_minus | Increment/decrement. <br> `x++`, `x--` |
| question              | Ternary operator. <br> `x ? y : z` |
| colon                 | Type annotation or label. <br> `x: int` |
| arrow                 | Function return type or lambda. <br> `() -> int` |
| dot                   | Member access. <br> `obj.x` |
| at                    | Attribute or pointer. <br> `@aligned` or `int@` |
| left_paren/right_paren| Parentheses. <br> `(x + y)` |
| left_brace/right_brace| Braces for blocks. <br> `{ ... }` |
| left_bracket/right_bracket | Array or index. <br> `arr[0]` |
| semicolon             | Statement terminator. <br> `x = 5;` |
| comma                 | Separator. <br> `f(a, b)` |
| yield                 | Generator yield. <br> `yield x;` |
| async                 | Async function. <br> `async function f() {}` |
| await                 | Await expression. <br> `await f();` |
| error_token           | Error token for invalid lexemes. |
| end_of_file           | End of file marker. |

## Example
```tspp
let x = 42;
const PI = 3.14;
function add(a: int, b: int): int { return a + b; }
class MyClass {}
namespace Math {}
if (x > 0) { return x; }
#stack int y = 10;
```
