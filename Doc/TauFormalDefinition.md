# Tau Language Formal Definition

This document captures the concrete syntax of the Tau Interface Definition Language (IDL) as implemented by the lexer and parser found under `Include/KAI/Language/Tau` and `Source/Library/Language/Tau/Source`. The notation below uses an Extended Backus–Naur Form (EBNF) style:

- `[]` marks an optional element.
- `{}` marks zero or more repetitions.
- Terminals appear in single quotes.
- Non-terminals appear in angle brackets.

## Lexical Structure

Tau source text is a sequence of *tokens* produced by the Tau lexer.

### Keywords

```
namespace  class  interface  struct  enum  event  async  sync
const  Proxy  Agent
```

Keywords are reserved; they cannot be used where identifiers are expected.

### Identifiers

```
<identifier> ::= <letter-or-underscore> { <letter-or-digit-or-underscore> }
```

Identifiers may be qualified with `::` during parsing (for nested namespaces or type qualification) but are still emitted as individual identifier tokens by the lexer.

### Literals

```
<number-literal> ::= <digits> [ '.' <digits> ] [ <exponent> ]
<exponent> ::= ('e' | 'E') [ '+' | '-' ] <digits>
<string-literal> ::= '"' { <any-char-except-quote-or-newline> } '"'
```

### Punctuation and Operators

```
{ }  ( )  [ ]  ;  ,  =  .  :  ::
```

Line comments start with `//` and run to the end of the line. Whitespace, tabs, and newlines separate tokens.

## Grammar

The grammar below is intentionally permissive in the same spirit as the implementation, which prefers recovery over early failure. Semantic validation (such as ensuring symbols exist) occurs later during code generation.

```
<tau-file> ::= { <top-level-declaration> }

<top-level-declaration> ::= <namespace-declaration>
                         | <class-declaration>
                         | <interface-declaration>
                         | <struct-declaration>
                         | <enum-declaration>

<namespace-declaration> ::= 'namespace' <qualified-identifier> <namespace-body>
                          | 'namespace' <identifier> '=' <qualified-identifier> ';'

<namespace-body> ::= '{' { <top-level-declaration> } '}'

<qualified-identifier> ::= <identifier> { '::' <identifier> }

<class-declaration> ::= 'class' <identifier> [ <inheritance-clause> ] <class-body>
<inheritance-clause> ::= ':' <qualified-identifier>
<class-body> ::= '{' { <class-member> } '}'

<class-member> ::= <visibility-label>
                | <event-declaration>
                | <method-declaration>
                | <field-declaration>

<visibility-label> ::= ('public' | 'protected' | 'private') ':'

<interface-declaration> ::= 'interface' <identifier> <class-body>

<struct-declaration> ::= 'struct' <identifier> <struct-body>
<struct-body> ::= '{' { <struct-member> } '}'
<struct-member> ::= <method-declaration> | <field-declaration>

<enum-declaration> ::= 'enum' <identifier> <enum-body>
<enum-body> ::= '{' { <enum-element> [ ',' ] } '}'
<enum-element> ::= <identifier> [ '=' <number-literal> ]

<event-declaration> ::= 'event' <identifier> '(' [ <parameter-list> ] ')' [ ';' ]

<method-declaration> ::= [ 'static' ] [ 'async' | 'sync' ] <type-name> <identifier>
                          '(' [ <parameter-list> ] ')' [ 'const' ] [ ';' ]

<field-declaration> ::= [ 'static' ] [ 'const' ] <type-name> <declarator> [ ';' ]
<declarator> ::= <identifier> [ <array-suffix> ] [ '=' <field-initialiser> ]
<array-suffix> ::= '[' ']'

<parameter-list> ::= <parameter> { ',' <parameter> }
<parameter> ::= <type-name> [ <array-suffix> ] <identifier> [ '=' <expression> ]

<type-name> ::= <qualified-identifier> [ <generic-arguments> ]
<generic-arguments> ::= '<' <type-name> { ',' <type-name> } '>'

<field-initialiser> ::= <expression>

<expression> ::= <number-literal>
               | <string-literal>
               | <qualified-identifier>

```

### Notes

1. The implementation currently tokenises nested namespace separators `::` as repeated `':'` characters that map to the `Semi` token. The grammar captures the intended structure where `::` denotes qualified identifiers.
2. `static`, `const`, `async`, and `sync` are parsed leniently; unsupported combinations are ignored by later stages.
3. Array suffixes are limited to a single `[]` in the parser.
4. Default values accept numeric, string, or identifier expressions. Dotted identifiers support enum member references such as `ConnectionState.Disconnected`.
5. Template or generic arguments are lexed as identifiers; the grammar shows the intended structure even though the lexer currently stores the entire `Vector<T>` token as a single identifier.

This formal definition should be used as the authoritative reference when writing Tau IDL or extending the language and its tooling.
