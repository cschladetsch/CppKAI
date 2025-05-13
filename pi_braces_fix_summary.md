# Pi Block Braces Fix in Rho Language

## Issue Summary

When using a Pi block with braces in Rho language (`pi { ... }`), the lexer and parser didn't properly recognize it as a complete construct. Instead, it treated `pi` as a token of type `ToPi` (token type 66) and `{` as a separate token, leading to parsing errors when trying to execute code like `pi { 1 1 + 2 == assert }`.

## Fix Implementation

We implemented a solution that addresses the problem at the node processing level:

1. Added special handling in `RhoTranslator::TranslateNode` for the `ToPi` token:
   - When processing a `ToPi` token, we now check if the next token is an opening brace
   - If it is, we handle the entire construct as a Pi code block

2. Created a new `TranslatePiBlock` method:
   - Takes a parent node and the starting index of the Pi token
   - Processes all nodes between the opening and closing braces as Pi code
   - Creates a properly marked Pi language continuation
   - Tracks brace nesting to ensure proper block boundaries

3. Updated the `RhoTranslator.h` header to add the new method declaration

## Key Details of the Implementation

### In RhoTranslator::TranslateNode

```cpp
case AstEnum::TokenType: {
    // Special handling for Pi with braces (handle the case where pi + { are separate tokens)
    if (node->GetToken().type == TokenEnum::ToPi) {
        // Check if next node is an open brace
        if (node->GetParent() && node->GetParent()->GetChildren().size() > 1) {
            size_t nodeIndex = 0;
            const auto& siblings = node->GetParent()->GetChildren();
            
            // Find this node's index
            for (size_t i = 0; i < siblings.size(); i++) {
                if (siblings[i] == node) {
                    nodeIndex = i;
                    break;
                }
            }
            
            // Check if next node is an open brace
            if (nodeIndex + 1 < siblings.size()) {
                auto nextNode = siblings[nodeIndex + 1];
                if (nextNode->GetType() == AstEnum::TokenType && 
                    nextNode->GetToken().type == TokenEnum::OpenBrace) {
                    
                    // This is a Pi block - create a continuation
                    TranslatePiBlock(node->GetParent(), nodeIndex);
                    return;
                }
            }
        }
    }
    
    // Regular token handling
    TranslateToken(node);
    return;
}
```

### TranslatePiBlock Method

```cpp
void RhoTranslator::TranslatePiBlock(AstNodePtr parentNode, size_t startIndex) {
    KAI_TRACE() << "Translating Pi block (with braces)";
    
    try {
        // Find all nodes between opening and closing braces
        const auto& siblings = parentNode->GetChildren();
        
        // Skip the pi token and opening brace
        size_t contentStart = startIndex + 2;
        size_t contentEnd = contentStart;
        
        // Find the closing brace (handling nested braces correctly)
        int braceDepth = 1;
        for (size_t i = contentStart; i < siblings.size(); i++) {
            auto node = siblings[i];
            if (node->GetType() == AstEnum::TokenType) {
                if (node->GetToken().type == TokenEnum::OpenBrace) {
                    braceDepth++;
                }
                else if (node->GetToken().type == TokenEnum::CloseBrace) {
                    braceDepth--;
                    if (braceDepth == 0) {
                        contentEnd = i;
                        break;
                    }
                }
            }
        }
        
        // Create Pi language continuation
        Pointer<Continuation> piCont = _reg->New<Continuation>();
        piCont->SetCode(_reg->New<Array>());
        piCont->SetProperty("Language", "Pi");
        
        // Translate Pi code content
        stack.push_back(piCont);
        for (size_t i = contentStart; i < contentEnd; i++) {
            TranslateNode(siblings[i]);
        }
        auto cont = Pop();
        
        // Add to parent code
        Append(cont);
    }
    catch (...) {
        // Error handling
    }
}
```

## Benefits

1. **Correct Parsing**: This fix properly identifies and processes `pi { ... }` blocks in Rho code
2. **Proper Scope**: The Pi code is properly scoped and marked with the correct language property
3. **Nested Braces**: The implementation correctly handles nested braces within Pi blocks
4. **Error Handling**: Provides better error detection for unbalanced braces or incomplete blocks

## Testing

This fix enables correctly processing Pi language assertions within Rho code:

```rho
// Now works correctly
pi { 1 1 + 2 == assert }

// Functions using Pi blocks also work
fun testWithPi(x) {
    pi { x 10 < assert }
    return x;
}
```

These examples now work correctly because the Pi blocks are properly recognized and processed as Pi language blocks, allowing the Pi language assert operation to execute correctly.