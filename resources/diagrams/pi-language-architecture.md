# Pi Language Architecture

## Pi Stack-Based Execution Model

```mermaid
graph TB
    subgraph "Pi Source Code"
        SRC[Pi Source<br/>2 3 + dup *<br/>{ 'square # } &]
    end
    
    subgraph "Lexical Analysis"
        LEX[Pi Lexer<br/>Tokenization]
        TOKENS[Token Stream<br/>NUMBER, NUMBER, PLUS<br/>DUP, MULTIPLY, etc.]
    end
    
    subgraph "Syntax Analysis"
        PAR[Pi Parser<br/>AST Construction]
        AST[Pi AST Nodes<br/>Operation nodes<br/>Literal nodes<br/>Continuation nodes]
    end
    
    subgraph "Translation"
        TRANS[Pi Translator<br/>AST → Continuations]
        CONT[Continuation Objects<br/>Executable code blocks]
    end
    
    subgraph "Execution Environment"
        EXE[Executor<br/>Stack-based VM]
        DS[Data Stack<br/>Values & Objects]
        CS[Context Stack<br/>Continuations & Control]
        REG[Registry<br/>Object Factory<br/>Type System]
    end
    
    SRC --> LEX
    LEX --> TOKENS
    TOKENS --> PAR
    PAR --> AST
    AST --> TRANS
    TRANS --> CONT
    CONT --> EXE
    EXE <--> DS
    EXE <--> CS
    EXE <--> REG
    
    style SRC fill:#e3f2fd
    style EXE fill:#4caf50
    style DS fill:#ff9800
    style CS fill:#9c27b0
    style REG fill:#2196f3
```

## Pi Stack Operations and Data Flow

```mermaid
sequenceDiagram
    participant User as User Input
    participant Lexer as Pi Lexer
    participant Parser as Pi Parser
    participant Trans as Pi Translator
    participant Exec as Executor
    participant DataStack as Data Stack
    participant ContextStack as Context Stack
    
    User->>Lexer: "2 3 + dup *"
    Lexer->>Parser: [NUMBER(2), NUMBER(3), PLUS, DUP, MULTIPLY]
    Parser->>Parser: Build AST nodes
    Parser->>Trans: AST tree
    Trans->>Trans: Convert to continuations
    Trans->>Exec: Executable continuation
    
    Exec->>DataStack: Push 2
    Exec->>DataStack: Push 3
    DataStack->>Exec: Pop 3, Pop 2
    Exec->>DataStack: Push 5 (2+3)
    Exec->>DataStack: Duplicate top (5)
    DataStack->>Exec: Pop 5, Pop 5
    Exec->>DataStack: Push 25 (5*5)
    
    Exec->>User: Result: 25
```

## Pi Component Architecture

```mermaid
classDiagram
    class PiLexer {
        +Tokenize(source): TokenStream
        +GetNextToken(): Token
        +HandleNumber(): NumberToken
        +HandleIdentifier(): IdentifierToken
        +HandleString(): StringToken
        -currentPos: int
        -source: string
    }
    
    class PiParser {
        +Parse(tokens): AstNode
        +ParseExpression(): AstNode
        +ParseContinuation(): ContinuationNode
        +ParseLiteral(): LiteralNode
        -tokens: TokenStream
        -currentToken: Token
    }
    
    class PiTranslator {
        +Translate(ast): Continuation
        +TranslateLiteral(node): void
        +TranslateOperation(node): void
        +TranslateContinuation(node): void
        -executor: Executor
        -registry: Registry
    }
    
    class PiAstNode {
        +Type type
        +Token token
        +Children children[]
        +Accept(visitor): void
        +ToString(): string
    }
    
    class OperationNode {
        +Operation operation
        +StackEffect effect
        +Execute(): void
    }
    
    class ContinuationNode {
        +Body statements[]
        +CreateContinuation(): Continuation
    }
    
    class LiteralNode {
        +Value value
        +DataType type
    }
    
    PiLexer --> PiParser : Token Stream
    PiParser --> PiTranslator : AST
    PiAstNode <|-- OperationNode
    PiAstNode <|-- ContinuationNode
    PiAstNode <|-- LiteralNode
    PiParser --> PiAstNode : Creates
    PiTranslator --> OperationNode : Processes
    PiTranslator --> ContinuationNode : Processes
    PiTranslator --> LiteralNode : Processes
```

## Pi Stack Machine Operations

```mermaid
graph LR
    subgraph "Stack Operations"
        PUSH[PUSH<br/>Add to top]
        POP[POP<br/>Remove from top]
        DUP[DUP<br/>Duplicate top]
        SWAP[SWAP<br/>Exchange top two]
        DROP[DROP<br/>Remove top]
        OVER[OVER<br/>Copy second to top]
    end
    
    subgraph "Arithmetic Operations"
        ADD[ADD<br/>Pop a,b Push a+b]
        SUB[SUB<br/>Pop a,b Push b-a]
        MUL[MUL<br/>Pop a,b Push a*b]
        DIV[DIV<br/>Pop a,b Push b/a]
        MOD[MOD<br/>Pop a,b Push b%a]
    end
    
    subgraph "Control Operations"
        IFE[IFE<br/>Conditional execution]
        CONT[CONTINUATION<br/>Code block creation]
        EXEC[EXEC (&)<br/>Execute continuation]
        SUSPEND[SUSPEND<br/>Pause execution]
        RESUME[RESUME<br/>Return to context]
    end
    
    subgraph "Variable Operations"
        STORE[STORE (#)<br/>Save to registry]
        FETCH[FETCH (@)<br/>Load from registry]
        ASSERT[ASSERT<br/>Validate condition]
        TRACE[TRACE<br/>Debug output]
    end
    
    style PUSH fill:#4caf50
    style ADD fill:#ff9800
    style IFE fill:#9c27b0
    style STORE fill:#2196f3
```

## Pi Continuation Model

```mermaid
graph TB
    subgraph "Continuation Creation"
        SRC_CONT[Source: { 2 3 + }]
        LEX_CONT[Lexer: Parse continuation body]
        AST_CONT[AST: Continuation node with body]
        TRANS_CONT[Translator: Create continuation object]
        CONT_OBJ[Continuation Object<br/>Executable code block]
    end
    
    subgraph "Continuation Execution"
        EXEC_CONT[& operator: Execute continuation]
        PUSH_CONTEXT[Push current context to context stack]
        EXEC_BODY[Execute continuation body operations]
        POP_CONTEXT[Pop context from context stack]
        RETURN[Return to caller]
    end
    
    subgraph "Continuation Control"
        SUSPEND_OP[SUSPEND: Pause and switch context]
        RESUME_OP[RESUME: Return to suspended context]
        REPLACE_OP[REPLACE: Substitute continuation]
    end
    
    SRC_CONT --> LEX_CONT --> AST_CONT --> TRANS_CONT --> CONT_OBJ
    CONT_OBJ --> EXEC_CONT
    EXEC_CONT --> PUSH_CONTEXT --> EXEC_BODY --> POP_CONTEXT --> RETURN
    
    EXEC_BODY -.-> SUSPEND_OP
    SUSPEND_OP -.-> RESUME_OP
    RESUME_OP -.-> REPLACE_OP
    
    style CONT_OBJ fill:#4caf50
    style EXEC_BODY fill:#ff9800
    style SUSPEND_OP fill:#9c27b0
```

## Pi Error Handling and Debugging

```mermaid
stateDiagram-v2
    [*] --> Parsing
    
    Parsing --> SyntaxError : Invalid syntax
    Parsing --> Translation : Valid AST
    
    Translation --> TranslationError : Invalid operations
    Translation --> Execution : Valid continuations
    
    Execution --> RuntimeError : Stack underflow/overflow
    Execution --> AssertionError : Failed assertion
    Execution --> Success : Normal completion
    Execution --> Debugging : Trace/Debug operations
    
    SyntaxError --> ErrorHandler
    TranslationError --> ErrorHandler
    RuntimeError --> ErrorHandler
    AssertionError --> ErrorHandler
    
    ErrorHandler --> [*] : Error reported
    Success --> [*] : Result returned
    Debugging --> Execution : Continue execution
```

## Pi Network Integration

```mermaid
graph LR
    subgraph "Local Pi Execution"
        LOCAL_PI[Pi Code<br/>2 3 +]
        LOCAL_EXEC[Local Executor<br/>Stack operations]
        LOCAL_RESULT[Result: 5]
    end
    
    subgraph "Network Layer"
        SERIALIZE[Serialize<br/>Pi operations]
        TRANSPORT[Network Transport<br/>RakNet/TCP]
        DESERIALIZE[Deserialize<br/>Remote operations]
    end
    
    subgraph "Remote Pi Execution"
        REMOTE_PI[Received Pi Code<br/>2 3 +]
        REMOTE_EXEC[Remote Executor<br/>Stack operations]
        REMOTE_RESULT[Result: 5]
    end
    
    LOCAL_PI --> LOCAL_EXEC --> LOCAL_RESULT
    LOCAL_PI --> SERIALIZE --> TRANSPORT --> DESERIALIZE --> REMOTE_PI
    REMOTE_PI --> REMOTE_EXEC --> REMOTE_RESULT
    REMOTE_RESULT --> SERIALIZE
    
    style LOCAL_EXEC fill:#4caf50
    style TRANSPORT fill:#ff9800
    style REMOTE_EXEC fill:#4caf50
```

## Pi Performance Characteristics

```mermaid
graph TB
    subgraph "Performance Metrics"
        PARSE_TIME[Parse Time<br/>~1ms for 1K tokens]
        TRANSLATE_TIME[Translation Time<br/>~2ms for complex AST]
        EXEC_TIME[Execution Time<br/>~0.1ms per operation]
        MEMORY_USAGE[Memory Usage<br/>~10KB per continuation]
    end
    
    subgraph "Optimization Features"
        DIRECT_EXEC[Direct Executor Mapping<br/>Minimal overhead]
        STACK_EFFICIENCY[Stack-based Operations<br/>No variable lookup]
        CONTINUATION_CACHE[Continuation Caching<br/>Reuse compiled code]
        INLINE_OPERATIONS[Inline Simple Operations<br/>Reduce call overhead]
    end
    
    PARSE_TIME -.-> DIRECT_EXEC
    TRANSLATE_TIME -.-> CONTINUATION_CACHE
    EXEC_TIME -.-> STACK_EFFICIENCY
    MEMORY_USAGE -.-> INLINE_OPERATIONS
    
    style DIRECT_EXEC fill:#4caf50
    style STACK_EFFICIENCY fill:#4caf50
    style CONTINUATION_CACHE fill:#2196f3
    style INLINE_OPERATIONS fill:#ff9800
```