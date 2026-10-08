#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Label.h>
#include <KAI/Core/Object/Object.h>

#include <functional>
#include <string>
#include <vector>

KAI_BEGIN

class Continuation;
class Tree;

/// PiNet: the rule a continuation must satisfy to be sent to another node.
/// It lives in CppKAI, not CppKaiCore: the Console app registers it with
/// Console::AddSendCheck, so Core and ConsoleLib never depend on it.
///
/// A continuation that travels may only use names it binds itself. Anything
/// it needs from the sender comes in on the stack, or is bound inside the
/// payload with `'name #`. Pi resolves a bare name dynamically (its own
/// scope, then each continuation on the context stack, then the tree), so
/// `{ a + }` would silently pick up whatever `a` means where it happens to
/// run. PiNet rejects it at the sender instead, with the names at fault.
///
/// What counts:
///
/// - bound: `'x #` (Store) anywhere in the continuation or in a block that
///   encloses it within the payload, and the continuation's formal arguments
///   (Rho and Sigma function parameters).
/// - used: a bare name `x` or `a/b` (resolved when run), and a quoted name
///   given to Retreive (`'x @`), Lookup, Assign or Remove.
/// - allowed without binding: system names, which every node has. With a
///   Tree, those are the children of /Bin, /Sys and /Types.
/// - never allowed: an absolute path outside /Bin, /Sys and /Types, and the
///   operations that reach into the running node itself (this, self, the
///   current scope and context, `cd`, `ls`, the executor, ExecFile).
///
/// A quoted name used only as data, such as a peer for `send`, is neither.
/// The check is flow-insensitive: a name bound anywhere in its block counts
/// as bound throughout it.
struct PiNet {
    struct Report {
        std::vector<std::string> unbound;      // used, but neither bound nor system names
        std::vector<std::string> environment;  // operations and paths that reach into the sender
        [[nodiscard]] bool Ok() const { return unbound.empty() && environment.empty(); }
        /// One line, e.g. "uses 'a', 'b', which it does not bind; uses 'this', which ..."
        [[nodiscard]] std::string ToString() const;
    };

    /// True for names every node provides.
    using SystemNames = std::function<bool(const Label &)>;

    /// Check a continuation and every continuation nested in its code.
    static Report Check(const Continuation &cont, const SystemNames &system = {});

    /// Check whatever is about to be sent: a Continuation, an Array of
    /// values (checked element by element), or a frozen BinaryStream (thawed
    /// to look inside, then rewound). Other values are plain data and pass.
    static Report Check(Object payload, const Tree *tree);

    /// Throw if `payload` fails Check(payload, tree). The message starts
    /// with "PiNet:" and names what is at fault.
    static void Require(Object payload, const Tree *tree);

    /// The children of /Bin, /Sys and /Types in `tree`.
    static SystemNames SystemNamesOf(const Tree *tree);
};

KAI_END
