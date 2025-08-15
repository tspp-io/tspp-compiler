Program            ::= { Import | Declaration | Statement }

/* ---------- Imports ---------- */
Import             ::= 'import' Identifier [ 'from' StringLiteral ] ';'

/* ---------- Declarations ---------- */
Declaration        ::= VariableDecl | FunctionDecl | ClassDecl | InterfaceDecl | TypeAliasDecl

/* ---------- Types & Memory ---------- */
VariableDecl       ::= { VarModifier } ('let' | 'const') Identifier ':' Type [ '=' Expression ] ';'
VarModifier        ::= '#' ('stack' | 'heap' | 'static')

Type               ::= UnionType
UnionType          ::= IntersectionType { '|' IntersectionType }
IntersectionType   ::= PrimaryType { '&' PrimaryType }
PrimaryType        ::= BasicType
                    | PointerType
                    | SmartPointerType
                    | Identifier [ TypeArguments ]
                    | '(' Type ')'            /* grouping */

BasicType          ::= 'int' | 'float' | 'bool' | 'string'
PointerType        ::= [ '@' ('unsafe' | 'aligned') ] Type '*'
SmartPointerType   ::= '#' ('shared' | 'unique' | 'weak') '<' Type '>'

TypeArguments      ::= '<' Type { ',' Type } '>'
TypeParameters     ::= '<' TypeParam { ',' TypeParam } '>'
TypeParam          ::= Identifier [ 'extends' Type ]

/* ---------- Functions ---------- */
FunctionDecl       ::= { FunctionModifier } 'function' Identifier '(' [ ParameterList ] ')'
                        [ ':' Type ] Block
FunctionModifier   ::= '#' ('const' | 'constexpr' | 'zerocast' | 'simd' | 'prefetch' | 'atomic' | 'pinned')

ParameterList      ::= Parameter { ',' Parameter }
Parameter          ::= Identifier [ ':' Type ] [ '=' Expression ]   /* TS-like (name: type = default) */

/* ---------- Classes & Interfaces ---------- */
ClassDecl          ::= { ClassModifier } 'class' Identifier [ TypeParameters ]
                        [ 'extends' Identifier [ TypeArguments ] ]
                        [ 'implements' Identifier [ TypeArguments ] { ',' Identifier [ TypeArguments ] } ]
                        ClassBody

ClassModifier      ::= '#' ('abstract' | 'packed' | 'pinned' | 'final')

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

/* ---------- Interfaces ---------- */
InterfaceDecl      ::= 'interface' Identifier [ TypeParameters ] InterfaceBody
InterfaceBody      ::= '{' { InterfaceMember } '}'
InterfaceMember    ::= MethodSignature | FieldSignature
MethodSignature    ::= Identifier '(' [ ParameterList ] ')' [ ':' Type ] ';'
FieldSignature     ::= Identifier ':' Type ';'

/* ---------- Type Aliases ---------- */
/* Prefer TS-style keyword */
TypeAliasDecl      ::= 'typedef' Identifier '=' Type ';'

/* ---------- Statements ---------- */
Statement          ::= Block | ExpressionStatement | IfStatement | WhileStatement | ForStatement | ReturnStatement

Block              ::= '{' { Statement } '}'
ExpressionStatement::= Expression ';'

IfStatement        ::= 'if' '(' Expression ')' Statement [ 'else' Statement ]
WhileStatement     ::= 'while' '(' Expression ')' Statement

ForStatement       ::= 'for' '(' ForInit ';' [ Expression ] ';' [ Expression ] ')' Statement
ForInit            ::= /* empty */
                    | VariableDecl
                    | Expression

ReturnStatement    ::= 'return' [ Expression ] ';'

/* ---------- Expressions (with precedence) ---------- */
/* Right-associative assignment and clear precedence tiers */

Expression         ::= Assignment

Assignment         ::= LogicalOr
                    | Assignable '=' Assignment

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

Unary              ::= { ('!' | '+' | '-' | '~') } Postfix

Postfix            ::= Primary { PostfixOp }
PostfixOp          ::= CallArgs
                    | MemberDot
                    | IndexBracket
CallArgs           ::= '(' [ ArgumentList ] ')'
MemberDot          ::= '.' Identifier
IndexBracket       ::= '[' Expression ']'

Primary            ::= Literal
                    | Identifier
                    | This
                    | NewExpr
                    | '(' Expression ')'

NewExpr            ::= 'new' Identifier [ TypeArguments ] '(' [ ArgumentList ] ')'

MemberAccess       ::= Primary MemberDot { PostfixOp }   /* e.g., obj.method().x */
IndexAccess        ::= Primary IndexBracket { PostfixOp }

ArgumentList       ::= Expression { ',' Expression }

Literal            ::= Number | StringLiteral | Boolean | Null
Boolean            ::= 'true' | 'false'
Null               ::= 'null'
This               ::= 'this'

/* ---------- Operators (token set; semantics by tiers above) ---------- */
OperatorTokenSet   ::= '+' | '-' | '*' | '/' | '%'
                    | '==' | '!=' | '<' | '>' | '<=' | '>='
                    | '&&' | '||' | '!'
                    | '&' | '|' | '^' | '<<' | '>>'
                    | '~' | '='
