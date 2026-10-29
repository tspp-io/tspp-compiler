# tspp Formal Grammar (EBNF)

This is the grammar the parser accepts (`src/parser/visitors/ast_builder/`).
Accepted is not the same as implemented: many constructs parse but have no
behavior yet, or are rejected later with a "not yet supported by code
generation" error. Notes in `/* ... */` say which. For what runs today, see
the "Works today" table in the [README](../README.md#works-today).

Statements end at `;` or at the end of a line: the lexer inserts a `;` at a
line end unless the line obviously continues (after `,`, `(` or a binary
operator, or when the next line starts with `)`, `]`, `.` or a binary
operator).

## Program Structure

```ebnf
Program            ::= { Import | Declaration | Statement }
```

---

## Imports

```ebnf
Import             ::= 'import' Identifier [ 'from' StringLiteral ] ';'
                       /* 'import X from "./file"' merges file.tspp into this
                          program (no namespaces; X is ignored). 'import sys;'
                          names the built-in sys.* runtime calls.
                          'import { x } from ...' is not supported. */
```

---

## Declarations

```ebnf
Declaration        ::= VariableDecl | FunctionDecl | ClassDecl | InterfaceDecl | TypeAliasDecl
```

---

## Types & Memory

```ebnf
VariableDecl       ::= [ VarModifier ] ('let' | 'const') Identifier [ ':' Type ] [ '=' Expression ] ';'
VarModifier        ::= '#' ('stack' | 'heap' | 'static')
                       /* parsed; '#heap' and '#stack' have no effect yet,
                          '#static' gives a global internal linkage */

Type               ::= UnionType
UnionType          ::= IntersectionType { '|' IntersectionType }
IntersectionType   ::= PrimaryType { '&' PrimaryType }
PrimaryType        ::= BasicType
                    | PointerType
                    | SmartPointerType
                    | Identifier [ TypeArguments ]
                    | '(' Type ')'            /* grouping */

BasicType          ::= 'int' | 'float' | 'double' | 'boolean' | 'bool' | 'string' | 'void'
                       /* one table: src/tokens/primitive_types.h.
                          'boolean' and 'bool' are the same type (LLVM i1).
                          'int' is 32-bit; 'float' values currently print
                          wrong (known bug). */
PointerType        ::= [ '@' ('unsafe' | 'aligned') ] Type '*'
                       /* parsed; no pointer semantics yet (planned) */
SmartPointerType   ::= '#' ('shared' | 'unique' | 'weak') '<' Type '>'
                       /* planned: does not parse yet ('#shared' lexes as one
                          token the type parser does not expect), and '>>'
                          closing nested generics lexes as a shift */

TypeArguments      ::= '<' Type { ',' Type } '>'
TypeParameters     ::= '<' TypeParam { ',' TypeParam } '>'
TypeParam          ::= Identifier [ 'extends' Type ]
```

---

## Functions

```ebnf
FunctionDecl       ::= { FunctionModifier } 'function' Identifier '(' [ ParameterList ] ')'
                        [ ':' Type ] Block
FunctionModifier   ::= '#' ('const' | 'constexpr' | 'zerocast' | 'simd' | 'prefetch' | 'atomic' | 'pinned')
                       /* recorded as attributes; no effect yet */

ParameterList      ::= Parameter { ',' Parameter }
Parameter          ::= Identifier [ ':' Type ] [ '=' Expression ]
                       /* TS-like (name: type = default) */
```

---

## Classes & Interfaces

```ebnf
ClassDecl          ::= { ClassModifier } 'class' Identifier [ TypeParameters ]
                        [ 'extends' Identifier [ TypeArguments ] ]
                        [ 'implements' Identifier [ TypeArguments ]
                          { ',' Identifier [ TypeArguments ] } ]
                        ClassBody

ClassModifier      ::= '#' ('abstract' | 'packed' | 'pinned' | 'final')
                       /* parsed (tests/modifiers/); no effect yet */

ClassBody          ::= '{' { ClassMember } '}'
ClassMember        ::= FieldDecl | MethodDecl | ConstructorDecl | StaticBlock

FieldDecl          ::= { AccessModifier } { FieldModifier }
                        ('let' | 'const') Identifier [ ':' Type ] [ '=' Expression ] ';'
FieldModifier      ::= '#' ('static' | 'readonly' | 'volatile' | 'atomic' | 'constexpr' | 'heap' | 'stack')
AccessModifier     ::= 'public' | 'private' | 'protected'

MethodDecl         ::= { AccessModifier } { MethodModifier }
                        'function' Identifier '(' [ ParameterList ] ')'
                        [ ':' Type ] Block
MethodModifier     ::= '#' ('static' | 'abstract' | 'virtual' | 'override' | 'constexpr' | 'inline' | 'atomic')

ConstructorDecl    ::= { AccessModifier } 'constructor' '(' [ ParameterList ] ')' Block
StaticBlock        ::= 'static' Block
```

---

## Interfaces

```ebnf
InterfaceDecl      ::= 'interface' Identifier [ TypeParameters ] InterfaceBody
InterfaceBody      ::= '{' { InterfaceMember } '}'
InterfaceMember    ::= MethodSignature | FieldSignature
MethodSignature    ::= Identifier '(' [ ParameterList ] ')' [ ':' Type ] ';'
FieldSignature     ::= Identifier ':' Type ';'
```

---

## Type Aliases

```ebnf
TypeAliasDecl      ::= ('typedef' | 'type') Identifier [ TypeParameters ] '=' Type ';'
                       /* union and object types are parsed but collapse to
                          a single type in the checker */
```

---

## Statements

```ebnf
Statement          ::= Block | VariableDecl | ExpressionStatement | IfStatement
                     | WhileStatement | ForStatement | ReturnStatement
                     | AsmStatement | NotLoweredStatement

/* Top-level statements are allowed; they run in source order before main. */

Block              ::= '{' { Statement } '}'
ExpressionStatement::= Expression ';'

IfStatement        ::= 'if' '(' Expression ')' Statement [ 'else' Statement ]
WhileStatement     ::= 'while' '(' Expression ')' Statement
                       /* conditions must be boolean today: an int condition
                          is a known bug (internal compiler error) */

ForStatement       ::= 'for' '(' ForInit ';' [ Expression ] ';' [ Expression ] ')' Statement
ForInit            ::= /* empty */
                    | VariableDecl
                    | Expression

ReturnStatement    ::= 'return' [ Expression ] ';'

AsmStatement       ::= '#asm' '(' StringLiteral [ ':' Operands [ ':' Operands [ ':' Clobbers ] ] ] ')' ';'
                       /* GCC-style inline assembly, lowered to LLVM inline asm
                          (used by lib/system.tspp; no passing test yet) */

/* Parsed and checked, then rejected with "'<kind>' is parsed but not yet
   supported by code generation": */
NotLoweredStatement::= 'switch' | 'do' ... 'while' | 'for' '(' ... 'of' ... ')'
                     | 'break' | 'continue' | 'try' ... | 'throw' Expression

/* 'enum' and 'namespace' declarations are rejected ("not supported yet"). */
```

---

## Expressions (with precedence)

```ebnf
/* Right-associative assignment and clear precedence tiers */

Expression         ::= Assignment

/* Loosest to tightest (TypeScript/C order; ExpressionBuilder::getOperatorPrecedence):
     = op=  <  ?:  <  ||  <  &&  <  |  <  ^  <  &  <  == !=  <  < <= > >=
            <  << >>  <  + -  <  * / %  <  unary
   Assignment and ?: are right-associative; all other binary operators are
   left-associative. Tested by tests/regression/h3_precedence_table.tspp. */

Assignment         ::= Conditional
                    | Assignable AssignOp Assignment
AssignOp           ::= '=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=' | '^='

Conditional        ::= LogicalOr [ '?' Assignment ':' Assignment ]

Assignable         ::= Identifier
                    | MemberAccess
                    | IndexAccess
                    /* (Optionally) unary '*' Expression for pointer-deref LHS if supported */

LogicalOr          ::= LogicalAnd { '||' LogicalAnd }
LogicalAnd         ::= BitwiseOr  { '&&' BitwiseOr  }

BitwiseOr          ::= BitwiseXor { '|'  BitwiseXor }
BitwiseXor         ::= BitwiseAnd { '^'  BitwiseAnd }
BitwiseAnd         ::= Equality   { '&'  Equality   }

Equality           ::= Relational { ('==' | '!=') Relational }
Relational         ::= Shift      { ('<' | '>' | '<=' | '>=') Shift }
Shift              ::= Additive   { ('<<' | '>>') Additive }
Additive           ::= Multiplicative { ('+' | '-') Multiplicative }
Multiplicative     ::= Unary      { ('*' | '/' | '%') Unary }

Unary              ::= ('!' | '+' | '-' | '~' | '@' | '*' | '++' | '--') Unary
                    | Postfix
                    /* '@' (address-of) and '*' (dereference) parse; they
                       have no tested semantics yet */

Postfix            ::= Primary { PostfixOp } [ '++' | '--' ]
PostfixOp          ::= CallArgs
                    | MemberDot
                    | IndexBracket
CallArgs           ::= '(' [ ArgumentList ] ')'
MemberDot          ::= '.' Identifier
IndexBracket       ::= '[' Expression ']'

Primary            ::= Literal
                    | Identifier
                    | ArrayLiteral
                    | This
                    | NewExpr
                    | '(' Expression ')'

NewExpr            ::= 'new' Identifier [ TypeArguments ] '(' [ ArgumentList ] ')'
                    | 'new' Type '[' Expression ']'
ArrayLiteral       ::= '[' [ ArgumentList ] ']'   /* int elements only */

MemberAccess       ::= Primary MemberDot { PostfixOp }   /* e.g., obj.method().x */
IndexAccess        ::= Primary IndexBracket { PostfixOp }

ArgumentList       ::= Expression { ',' Expression }

Literal            ::= Number | StringLiteral | Boolean | Null
Boolean            ::= 'true' | 'false'
Null               ::= 'null'
This               ::= 'this'
```

---

## Operators (token set)

```ebnf
OperatorTokenSet   ::= '+' | '-' | '*' | '/' | '%'
                    | '==' | '!=' | '<' | '>' | '<=' | '>='
                    | '&&' | '||' | '!'
                    | '&' | '|' | '^' | '<<' | '>>'
                    | '~' | '=' | '?' | ':'
                    | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=' | '^='
                    | '++' | '--'
```
