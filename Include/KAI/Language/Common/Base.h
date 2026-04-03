#pragma once

// This is the basis for the language-agnostic execution system.
//
// There is one Translator<> type, that uses a combination of
// each of these other templates to transform a text string
// into an executable stream for an Executor.
//
// It's a bit too involved to try and explain it with just text.
//
// The good news is that you currently have the code itself...

KAI_BEGIN

// A Token given the Abstract Syntax Tree node type that it will
// have to make
template <class AstEnumStruct>
class TokenBase;

// a Lexer given the token stream it will use
template <class Token>
class LexerCommon;

// a node in the AST tree_ given the AST type
// TODO: rename to AstCommon. Current tools make this hard.
template <class AstEnum>
class NodeBase;

// Base class for a specialised parser, given a general lexer and Ast node type
template <class Lexer, class AstEnumStruct>
class ParserCommon;

KAI_END
