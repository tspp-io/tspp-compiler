# TSPP++ Formal Grammar (EBNF)

## Program Structure
```
Program        ::= { Import | Declaration | Statement }
Import         ::= 'import' Identifier [ 'from' StringLiteral ] ';'
Declaration    ::= VariableDecl | FunctionDecl | ClassDecl | InterfaceDecl | TypeAliasDecl
Statement      ::= Block | ExpressionStatement | IfStatement | WhileStatement | ForStatement | ReturnStatement | ...
```

## Memory and Types
```
VariableDecl   ::= [ '#' ('stack' | 'heap' | 'static') ]  Identifier:Type [ '=' Expression ] ';'
Type           ::= BasicType | PointerType | SmartPointerType | UnionType | TypeConstraint
BasicType      ::= 'int' | 'float' | 'bool' | 'string' | Identifier
PointerType    ::= [ '@' ('unsafe' | 'aligned') ] Type '*'
SmartPointerType ::= '#' ('shared' | 'unique' | 'weak') '<' Type '>'
UnionType      ::= Type '|' Type { '|' Type }
TypeConstraint ::= Type [ 'extends' Type ]
```

## Functions
```
FunctionDecl   ::= [ FunctionModifier ] 'function' Identifier '(' [ ParameterList ] ')' [ ':' Type ] Block
FunctionModifier       ::= '#' ('const' | 'constexpr' | 'zerocast' | 'simd' | 'prefetch' | 'atomic' | 'pinned')
ParameterList  ::= Parameter { ',' Parameter }
Parameter      ::= Type Identifier
```

## Classes and Interfaces
```
ClassDecl        ::= ClassModifiers 'class' Identifier [ 'extends' Identifier ] Block
ClassModifier  ::= '#' ('abstract' | 'packed' | 'pinned')
InterfaceDecl  ::= 'interface' Identifier Block
TypeAliasDecl  ::= 'typedef' Identifier '=' Type ';'
```

## Statements
```
Block          ::= '{' { Statement } '}'
ExpressionStatement ::= Expression ';'
IfStatement    ::= 'if' '(' Expression ')' Statement [ 'else' Statement ]
WhileStatement ::= 'while' '(' Expression ')' Statement
ForStatement   ::= 'for' '(' [ Expression ] ';' [ Expression ] ';' [ Expression ] ')' Statement
ReturnStatement ::= 'return' [ Expression ] ';'
```

## Expressions
```
Expression     ::= Assignment | Binary | Unary | Call | MemberAccess | Literal | Identifier
Assignment     ::= Identifier '=' Expression
Binary         ::= Expression Operator Expression
Unary          ::= Operator Expression
Call           ::= Identifier '(' [ ArgumentList ] ')'
MemberAccess   ::= Expression '.' Identifier
Literal        ::= Number | StringLiteral | Boolean | Null
ArgumentList   ::= Expression { ',' Expression }
```

## Operators
```
Operator       ::= '+' | '-' | '*' | '/' | '%' | '==' | '!=' | '<' | '>' | '<=' | '>=' | '&&' | '||' | '!' | ...
```

## Advanced Features
```
SmartPointer   ::= '#' ('shared' | 'unique' | 'weak') '<' Type '>'
FunctionModifier ::= '#' ('const' | 'constexpr' | 'zerocast' | 'simd' | 'prefetch' | 'atomic' | 'pinned')
PointerType    ::= [ '@' ('unsafe' | 'aligned') ] Type '*'
TypeConstraint ::= Type [ 'extends' Type ]
UnionType      ::= Type '|' Type { '|' Type }
```