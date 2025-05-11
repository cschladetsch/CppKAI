#!/bin/bash

# Path to TranslatorBase.h
TRANSLATOR_BASE_PATH="/home/xian/local/KAI/Include/KAI/Language/Common/TranslatorBase.h"

# Make a backup
cp "$TRANSLATOR_BASE_PATH" "${TRANSLATOR_BASE_PATH}.bak"

# Create a temporary file
TMP_FILE=$(mktemp)

# Replace the problematic Translate method
cat > "$TMP_FILE" << 'EOF'
    Pointer<Continuation> Translate(const char *text, Structure st) override {
        if (text == 0 || text[0] == 0) {
            KAI_TRACE_WARN_1("No input");
            return Object();
        }

        trace = 0;

        auto lex = std::make_shared<Lexer>(text, *_reg);
        lex->Process();
        if (lex->GetTokens().empty()) {
            KAI_TRACE_WARN_1("No tokens");
            return Object();
        }

        if (lex->Failed) {
            KAI_TRACE_WARN_1(lex->Error);
            Fail(lex->Error);
            return Object();
        }

        if (trace > 0) KAI_TRACE_1(lex->Print());

        auto parse = std::make_shared<Parser>(*_reg);
        parse->Process(lex, st);
        if (parse->Failed) {
            if (trace > 1) KAI_TRACE_1(parse->PrintTree());

            Fail(parse->Error);
            return Object();
        }

        if (trace > 1) KAI_TRACE_1(parse->PrintTree());

        // Create a single Program Continuation to hold all translated code
        Pointer<Continuation> program = _reg->New<Continuation>();
        program->SetCode(_reg->New<Array>());
        
        // Set it as our root translator element (no nesting/wrapping)
        stack.clear();
        stack.push_back(program);
        
        // Translate the AST directly into the program continuation
        TranslateNode(parse->GetRoot());
        
        // Return the program continuation directly - no unwrapping
        if (stack.empty()) KAI_THROW_0(EmptyStack);
        return stack[0];
    }
EOF

# Use sed to replace the Translate method
sed -e '/Pointer<Continuation> Translate(const char \*text, Structure st) override {/,/}/!b' \
    -e "/}/!d;r $TMP_FILE" \
    -e 'd' \
    "$TRANSLATOR_BASE_PATH.bak" > "$TRANSLATOR_BASE_PATH"

# Clean up
rm "$TMP_FILE"

echo "TranslatorBase.h updated to avoid Continuation wrapping"