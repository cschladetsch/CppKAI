#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Lisp/LispTranslator.h>

#include <stdexcept>
#include <string>
#include <vector>

KAI_BEGIN

void LispTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        Fail("Empty input");
        return;
    }

    switch (node->GetType()) {
        case AstNodeEnum::Program:
            TranslateProgram(node);
            return;
        case AstNodeEnum::TokenType:
            TranslateAtom(node);
            return;
        case AstNodeEnum::List:
            TranslateList(node);
            return;
        case AstNodeEnum::Quote:
            TranslateQuote(node);
            return;
        default:
            Fail("Unsupported Lisp AST node");
            return;
    }
}

void LispTranslator::TranslateProgram(AstNodePtr node) {
    for (auto const &child : node->GetChildren()) {
        if (Failed) return;
        TranslateNode(child);
    }
}

void LispTranslator::TranslateBody(AstNodePtr list, size_t from) {
    auto const &children = list->GetChildren();
    for (size_t i = from; i < children.size(); ++i) {
        if (Failed) return;
        TranslateNode(children[i]);
    }
}

void LispTranslator::TranslateAtom(AstNodePtr node) {
    auto const &token = node->GetToken();
    const std::string text = node->Text();

    switch (token.type) {
        case TokenEnum::Int:
            try {
                AppendNew<int>(std::stoi(text));
            } catch (std::exception const &) {
                Fail("Malformed integer literal: " + text);
            }
            return;

        case TokenEnum::Float:
            try {
                AppendNew<float>(std::stof(text));
            } catch (std::exception const &) {
                Fail("Malformed float literal: " + text);
            }
            return;

        case TokenEnum::String:
            Append(reg_->New<String>(String(text.c_str())));
            return;

        case TokenEnum::True:
            AppendNew<bool>(true);
            return;

        case TokenEnum::False:
            AppendNew<bool>(false);
            return;

        case TokenEnum::Nil:
            // nil is the empty list, as in most Lisps.
            Append(reg_->New<List>(List()));
            return;

        case TokenEnum::Ident:
            // A bare symbol evaluates to whatever it is bound to.
            Append(reg_->New<Label>(Label(text.c_str())));
            AppendDirectOperation(Operation::Retreive);
            return;

        default:
            Fail("Unexpected atom in Lisp source: " + text);
            return;
    }
}

void LispTranslator::TranslateQuote(AstNodePtr node) {
    if (node->GetChildren().empty()) {
        Fail("Nothing to quote");
        return;
    }

    auto quoted = node->GetChild(0);

    // Quoting a symbol yields the symbol itself rather than its value, which
    // is the whole point: no Retreive is emitted.
    if (quoted->GetType() == AstNodeEnum::TokenType &&
        quoted->GetToken().type == TokenEnum::Ident) {
        Append(reg_->New<Label>(Label(quoted->Text().c_str())));
        return;
    }

    // Quoting anything else that is not a list is just that value.
    if (quoted->GetType() != AstNodeEnum::List) {
        TranslateNode(quoted);
        return;
    }

    // A quoted list is data: build it at runtime from its quoted elements.
    for (auto const &element : quoted->GetChildren()) {
        if (Failed) return;
        auto wrapper = std::make_shared<AstNode>(AstNodeEnum::Quote);
        wrapper->Add(element);
        TranslateNode(wrapper);
    }

    AppendNew<int>(static_cast<int>(quoted->GetChildren().size()));
    AppendDirectOperation(Operation::ToList);
}

void LispTranslator::TranslateList(AstNodePtr node) {
    auto const &children = node->GetChildren();

    if (children.empty()) {
        // () is the empty list.
        Append(reg_->New<List>(List()));
        return;
    }

    auto head = children[0];
    if (head->GetType() == AstNodeEnum::TokenType &&
        head->GetToken().type == TokenEnum::Ident) {
        const std::string name = head->Text();
        if (TranslateSpecialForm(name, node)) return;
        if (TranslatePrimitive(name, node)) return;
    }

    TranslateCall(node);
}

void LispTranslator::StoreTo(std::string const &name) {
    // A leading quote marks the pathname as a name to bind rather than one to
    // resolve; this matches how Rho emits assignment.
    Append(reg_->New<Pathname>(Pathname(String(("'" + name).c_str()))));
    AppendDirectOperation(Operation::Store);
}

bool LispTranslator::ExpectArgs(AstNodePtr list, size_t count,
                                const char *form) {
    // +1 for the head symbol itself.
    if (list->GetChildren().size() != count + 1) {
        Fail(std::string(form) + " expects " + std::to_string(count) +
             " argument(s)");
        return false;
    }
    return true;
}

Pointer<Continuation> LispTranslator::MakeLambda(
    std::vector<AstNodePtr> const &params, AstNodePtr body, size_t bodyFrom) {
    PushNew();
    TranslateBody(body, bodyFrom);
    auto cont = Pop();

    // Args are declared in reverse so Continuation::Enter pops them off the
    // data stack back into left-to-right order.
    if (cont.Exists()) {
        for (auto it = params.rbegin(); it != params.rend(); ++it) {
            cont->AddArg(Label((*it)->Text().c_str()));
        }
    }

    return cont;
}

bool LispTranslator::TranslateSpecialForm(std::string const &head,
                                          AstNodePtr list) {
    auto const &children = list->GetChildren();
    const size_t argc = children.size() - 1;

    if (head == "quote") {
        if (!ExpectArgs(list, 1, "quote")) return true;
        auto wrapper = std::make_shared<AstNode>(AstNodeEnum::Quote);
        wrapper->Add(children[1]);
        TranslateNode(wrapper);
        return true;
    }

    if (head == "if") {
        if (argc != 2 && argc != 3) {
            Fail("if expects (if test then [else])");
            return true;
        }

        TranslateNode(children[1]);

        PushNew();
        TranslateNode(children[2]);
        auto thenCont = Pop();

        PushNew();
        if (argc == 3) TranslateNode(children[3]);
        auto elseCont = Pop();

        Append(thenCont);
        Append(elseCont);
        AppendDirectOperation(Operation::IfElse);
        return true;
    }

    if (head == "define" || head == "set!" || head == "defun") {
        if (argc < 2) {
            Fail(head + " expects a name and a value");
            return true;
        }

        auto target = children[1];

        // (define (name params...) body...)
        if (target->GetType() == AstNodeEnum::List) {
            auto const &sig = target->GetChildren();
            if (sig.empty()) {
                Fail("define needs a function name");
                return true;
            }

            std::vector<AstNodePtr> params(sig.begin() + 1, sig.end());
            auto cont = MakeLambda(params, list, 2);
            Append(cont);
            StoreTo(sig[0]->Text());
            return true;
        }

        // (define name value)
        if (!ExpectArgs(list, 2, head.c_str())) return true;
        TranslateNode(children[2]);
        StoreTo(target->Text());
        return true;
    }

    if (head == "lambda") {
        if (argc < 1) {
            Fail("lambda expects a parameter list");
            return true;
        }

        auto paramList = children[1];
        std::vector<AstNodePtr> params;
        if (paramList->GetType() == AstNodeEnum::List) {
            params.assign(paramList->GetChildren().begin(),
                          paramList->GetChildren().end());
        }

        Append(MakeLambda(params, list, 2));
        return true;
    }

    if (head == "begin" || head == "progn") {
        TranslateBody(list, 1);
        return true;
    }

    if (head == "let" || head == "let*") {
        if (argc < 1) {
            Fail("let expects a binding list");
            return true;
        }

        // Bindings are established in the enclosing scope in sequence, so this
        // is let* semantics; later bindings can see earlier ones.
        auto bindings = children[1];
        if (bindings->GetType() == AstNodeEnum::List) {
            for (auto const &binding : bindings->GetChildren()) {
                if (Failed) return true;
                if (binding->GetType() != AstNodeEnum::List ||
                    binding->GetChildren().size() != 2) {
                    Fail("let bindings must be (name value) pairs");
                    return true;
                }
                TranslateNode(binding->GetChild(1));
                StoreTo(binding->GetChild(0)->Text());
            }
        }

        TranslateBody(list, 2);
        return true;
    }

    if (head == "while") {
        if (argc < 1) {
            Fail("while expects a condition");
            return true;
        }

        PushNew();
        TranslateNode(children[1]);
        auto condCont = Pop();

        PushNew();
        TranslateBody(list, 2);
        auto bodyCont = Pop();

        Append(condCont);
        Append(bodyCont);
        AppendDirectOperation(Operation::WhileLoop);
        return true;
    }

    if (head == "cond") {
        TranslateCondFrom(list, 1);
        return true;
    }

    return false;
}

void LispTranslator::TranslateCondFrom(AstNodePtr list, size_t index) {
    auto const &children = list->GetChildren();

    // No clause matched and there was no else: leave nothing behind.
    if (index >= children.size()) return;

    auto clause = children[index];
    if (clause->GetType() != AstNodeEnum::List || clause->GetChildren().empty()) {
        Fail("cond clauses must be (test body...) lists");
        return;
    }

    auto test = clause->GetChild(0);
    const bool isElse = test->GetType() == AstNodeEnum::TokenType &&
                        test->GetToken().type == TokenEnum::Ident &&
                        test->Text() == "else";

    if (isElse) {
        TranslateBody(clause, 1);
        return;
    }

    TranslateNode(test);

    PushNew();
    TranslateBody(clause, 1);
    auto thenCont = Pop();

    PushNew();
    TranslateCondFrom(list, index + 1);
    auto elseCont = Pop();

    Append(thenCont);
    Append(elseCont);
    AppendDirectOperation(Operation::IfElse);
}

void LispTranslator::TranslateFold(AstNodePtr list, Operation::Type op,
                                   int identity, bool unaryUsesIdentity) {
    auto const &children = list->GetChildren();
    const size_t argc = children.size() - 1;

    if (argc == 0) {
        AppendNew<int>(identity);
        return;
    }

    if (argc == 1) {
        // (- x) negates and (/ x) reciprocates; (+ x) and (* x) are just x.
        if (unaryUsesIdentity) {
            AppendNew<int>(identity);
            TranslateNode(children[1]);
            AppendDirectOperation(op);
        } else {
            TranslateNode(children[1]);
        }
        return;
    }

    TranslateNode(children[1]);
    for (size_t i = 2; i < children.size(); ++i) {
        if (Failed) return;
        TranslateNode(children[i]);
        AppendDirectOperation(op);
    }
}

bool LispTranslator::TranslatePrimitive(std::string const &head,
                                        AstNodePtr list) {
    auto const &children = list->GetChildren();
    const size_t argc = children.size() - 1;

    // Variadic arithmetic, folded left-associatively.
    if (head == "+") return TranslateFold(list, Operation::Plus, 0, false), true;
    if (head == "*")
        return TranslateFold(list, Operation::Multiply, 1, false), true;
    if (head == "-")
        return TranslateFold(list, Operation::Minus, 0, true), true;
    if (head == "/")
        return TranslateFold(list, Operation::Divide, 1, true), true;
    if (head == "mod" || head == "%" || head == "modulo")
        return TranslateFold(list, Operation::Modulo, 0, false), true;

    // Short-circuiting is not modelled; these evaluate both sides.
    if (head == "and")
        return TranslateFold(list, Operation::LogicalAnd, 1, false), true;
    if (head == "or")
        return TranslateFold(list, Operation::LogicalOr, 0, false), true;

    if (head == "min") return TranslateFold(list, Operation::Min, 0, false), true;
    if (head == "max") return TranslateFold(list, Operation::Max, 0, false), true;

    struct Binary {
        const char *name;
        Operation::Type op;
    };
    static const Binary kBinary[] = {
        {"<", Operation::Less},         {">", Operation::Greater},
        {"<=", Operation::LessOrEquiv}, {">=", Operation::GreaterOrEquiv},
        {"=", Operation::Equiv},        {"==", Operation::Equiv},
        {"eq?", Operation::Equiv},      {"equal?", Operation::Equiv},
        {"/=", Operation::NotEquiv},    {"!=", Operation::NotEquiv},
    };

    for (auto const &entry : kBinary) {
        if (head != entry.name) continue;
        if (!ExpectArgs(list, 2, entry.name)) return true;
        TranslateNode(children[1]);
        TranslateNode(children[2]);
        AppendDirectOperation(entry.op);
        return true;
    }

    if (head == "not") {
        if (!ExpectArgs(list, 1, "not")) return true;
        TranslateNode(children[1]);
        AppendDirectOperation(Operation::LogicalNot);
        return true;
    }

    if (head == "print") {
        // Print pops a single value, so emit one per argument.
        for (size_t i = 1; i < children.size(); ++i) {
            if (Failed) return true;
            TranslateNode(children[i]);
            AppendDirectOperation(Operation::Print);
        }
        return true;
    }

    if (head == "assert") {
        if (!ExpectArgs(list, 1, "assert")) return true;
        TranslateNode(children[1]);
        AppendDirectOperation(Operation::Assert);
        return true;
    }

    if (head == "list") {
        for (size_t i = 1; i < children.size(); ++i) {
            if (Failed) return true;
            TranslateNode(children[i]);
        }
        AppendNew<int>(static_cast<int>(argc));
        AppendDirectOperation(Operation::ToList);
        return true;
    }

    if (head == "size" || head == "length") {
        if (!ExpectArgs(list, 1, head.c_str())) return true;
        TranslateNode(children[1]);
        AppendDirectOperation(Operation::Size);
        return true;
    }

    // Containers.
    //
    // This is an array-oriented Lisp rather than a cons-cell one: the runtime
    // stores Arrays, Lists and Maps with integer indexing, not pairs. car and
    // cdr still work because they fall out of Index and ArraySlice, but there
    // is no cons -- building a new list by prepending has no matching
    // operation, and nth/push!/set-nth! cover what cons would be used for.

    if (head == "array") {
        // (array v...) -- also the way to make an empty one, then grow it with
        // push!, which is how a large grid gets built.
        for (size_t i = 1; i < children.size(); ++i) {
            if (Failed) return true;
            TranslateNode(children[i]);
        }
        AppendNew<int>(static_cast<int>(argc));
        AppendDirectOperation(Operation::ToArray);
        return true;
    }

    if (head == "nth" || head == "at") {
        // (nth container index) -- container may be an array, map or string.
        if (!ExpectArgs(list, 2, head.c_str())) return true;
        TranslateNode(children[1]);
        TranslateNode(children[2]);
        AppendDirectOperation(Operation::Index);
        return true;
    }

    if (head == "set-nth!") {
        // (set-nth! array index value) -- leaves the array on the stack.
        if (!ExpectArgs(list, 3, "set-nth!")) return true;
        TranslateNode(children[1]);
        TranslateNode(children[2]);
        TranslateNode(children[3]);
        AppendDirectOperation(Operation::SetChild);
        // SetChild hands the container back; drop it so this reads as a
        // statement, the way Rho's a[i] = v does, instead of leaking a stack
        // entry on every call.
        AppendDirectOperation(Operation::Drop);
        return true;
    }

    if (head == "push!") {
        // (push! container value) -- leaves the container on the stack.
        if (!ExpectArgs(list, 2, "push!")) return true;
        TranslateNode(children[1]);
        TranslateNode(children[2]);
        AppendDirectOperation(Operation::ArrayPush);
        AppendDirectOperation(Operation::Drop);  // statement, as with set-nth!
        return true;
    }

    if (head == "slice") {
        if (!ExpectArgs(list, 3, "slice")) return true;
        TranslateNode(children[1]);
        TranslateNode(children[2]);
        TranslateNode(children[3]);
        AppendDirectOperation(Operation::ArraySlice);
        return true;
    }

    if (head == "car" || head == "first") {
        if (!ExpectArgs(list, 1, head.c_str())) return true;
        TranslateNode(children[1]);
        AppendNew<int>(0);
        AppendDirectOperation(Operation::Index);
        return true;
    }

    if (head == "cdr" || head == "rest") {
        // Everything from index 1 to the end. The container is translated
        // twice -- once to slice, once to measure -- which is fine for the
        // identifier or literal this is realistically applied to.
        if (!ExpectArgs(list, 1, head.c_str())) return true;
        TranslateNode(children[1]);
        AppendNew<int>(1);
        TranslateNode(children[1]);
        AppendDirectOperation(Operation::Size);
        AppendDirectOperation(Operation::ArraySlice);
        return true;
    }

    if (head == "dict") {
        // (dict k v k v ...) -- ToMap consumes a pair count, not an item count.
        if (argc % 2 != 0) {
            Fail("dict expects an even number of key/value arguments");
            return true;
        }
        for (size_t i = 1; i < children.size(); ++i) {
            if (Failed) return true;
            TranslateNode(children[i]);
        }
        AppendNew<int>(static_cast<int>(argc / 2));
        AppendDirectOperation(Operation::ToMap);
        return true;
    }

    if (head == "keys") {
        if (!ExpectArgs(list, 1, "keys")) return true;
        TranslateNode(children[1]);
        AppendDirectOperation(Operation::MapKeys);
        return true;
    }

    return false;
}

void LispTranslator::TranslateCall(AstNodePtr list) {
    auto const &children = list->GetChildren();

    // Arguments first, then the continuation to enter: the same shape Rho
    // emits, so Suspend finds the callee on top with its args beneath.
    for (size_t i = 1; i < children.size(); ++i) {
        if (Failed) return;
        TranslateNode(children[i]);
    }

    TranslateNode(children[0]);
    AppendDirectOperation(Operation::Suspend);
}

KAI_END
