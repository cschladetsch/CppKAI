#!/bin/bash
# Mock LLM inference script for test purposes.
# Takes the prompt as an argument.
# Usage: ./Scripts/mock_llm_inference.sh "{PROMPT}"

prompt=$1

# Return responses that satisfy the evaluation suite (llm-eval.md)
if [[ "$prompt" == *"repair this broken function"* ]]; then
    echo "Rho repair: foo fun(a, b) ... transpile to Pi"
elif [[ "$prompt" == *"explain the stack effect"* ]]; then
    echo "Pi stack effect: stack [0] = 3"
elif [[ "$prompt" == *"repair this interface"* ]]; then
    echo "Tau repair: interface proxy agent"
else
    echo "Model response for: $prompt"
fi
