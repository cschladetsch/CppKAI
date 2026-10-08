#include <KAI/Core/BinaryStream.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Pathname.h>
#include <KAI/Core/Tree.h>
#include <KAI/Executor/BinBase.h>
#include <KAI/Executor/Continuation.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/PiNet/PiNet.h>

#include <algorithm>
#include <set>

KAI_BEGIN

namespace {

// Operations that reach into the node running them rather than the payload.
bool ReachesIntoNode(Operation::Type op) {
    switch (op) {
        case Operation::This:
        case Operation::Self:
        case Operation::ThisContext:
        case Operation::ThisContinuation:
        case Operation::GetScope:
        case Operation::ChangeScope:
        case Operation::GetChildren:
        case Operation::Executor:
        case Operation::ExecFile: return true;
        default: return false;
    }
}

bool IsOperation(const Object &q, Operation::Type op) {
    return q.Exists() && q.IsType<Operation>() && ConstDeref<Operation>(q).GetTypeNumber() == op;
}

bool IsSystemRoot(const Label &first) {
    const std::string name = first.ToString().StdString();
    return name == "Bin" || name == "Sys" || name == "Types";
}

// A name as it appears in code: a Label, or a Pathname whose first element
// is what gets resolved.
struct Name {
    bool valid = false;
    bool quoted = false;
    bool absolute = false;
    Label first;       // the name resolved, for a relative name
    std::string text;  // for messages
};

Name NameOf(const Object &q) {
    Name n;
    if (!q.Exists()) return n;
    if (q.IsType<Label>()) {
        const Label &l = ConstDeref<Label>(q);
        n.valid = true;
        n.quoted = l.Quoted();
        n.first = l;
        n.first.SetQuoted(false);
        n.text = n.first.ToString().StdString();
        return n;
    }
    if (q.IsType<Pathname>()) {
        const Pathname &p = ConstDeref<Pathname>(q);
        n.valid = true;
        n.quoted = p.Quoted();
        n.absolute = p.Absolute();
        for (auto const &e : p.GetElements()) {
            if (e.type == Pathname::Element::Name) {
                n.first = e.name;
                break;
            }
        }
        n.text = p.ToString().StdString();
        if (!n.text.empty() && n.text.front() == '\'') n.text.erase(0, 1);
        if (!n.text.empty() && n.text.front() == '/') n.absolute = true;
        return n;
    }
    return n;
}

class Walker {
   public:
    Walker(const PiNet::SystemNames &system, PiNet::Report &report) : system_(system), report_(report) {}

    void Walk(const Continuation &cont) {
        scopes_.push_back(Binds(cont));
        if (cont.code.Exists()) {
            const Array &code = *cont.code;
            for (int i = 0; i < code.Size(); ++i) Item(code.At(i), i + 1 < code.Size() ? code.At(i + 1) : Object());
        }
        scopes_.pop_back();
    }

   private:
    const PiNet::SystemNames &system_;
    PiNet::Report &report_;
    std::vector<std::set<std::string>> scopes_;
    std::set<std::string> unbound_, environment_;

    // The names a block binds: `'x #` anywhere in its own code, and its
    // formal arguments. Nested blocks bind their own.
    static std::set<std::string> Binds(const Continuation &cont) {
        std::set<std::string> bound;
        if (cont.args.Exists())
            for (int i = 0; i < cont.args->Size(); ++i) {
                Name n = NameOf(cont.args->At(i));
                if (n.valid) bound.insert(n.first.ToString().StdString());
            }
        if (cont.code.Exists()) {
            const Array &code = *cont.code;
            for (int i = 0; i + 1 < code.Size(); ++i) {
                Name n = NameOf(code.At(i));
                if (n.valid && n.quoted && !n.absolute && IsOperation(code.At(i + 1), Operation::Store))
                    bound.insert(n.first.ToString().StdString());
            }
        }
        return bound;
    }

    void Item(const Object &q, const Object &next) {
        if (!q.Exists()) return;
        if (q.IsType<Continuation>()) {
            Walk(ConstDeref<Continuation>(q));
            return;
        }
        if (q.IsType<Operation>()) {
            const auto op = ConstDeref<Operation>(q).GetTypeNumber();
            if (ReachesIntoNode(op)) Environment(Operation::ToString(op));
            return;
        }
        Name n = NameOf(q);
        if (!n.valid) return;
        if (n.quoted) {
            // A quoted name is data unless it is looked up or assigned to.
            if (!IsOperation(next, Operation::Retreive) && !IsOperation(next, Operation::Lookup) &&
                !IsOperation(next, Operation::Assign) && !IsOperation(next, Operation::Remove))
                return;
        }
        if (n.absolute) {
            if (!IsSystemRoot(n.first)) Environment(n.text);
            return;
        }
        Use(n);
    }

    void Use(const Name &n) {
        const std::string name = n.first.ToString().StdString();
        if (name.empty()) return;
        for (auto const &scope : scopes_)
            if (scope.contains(name)) return;
        if (system_ && system_(n.first)) return;
        if (unbound_.insert(name).second) report_.unbound.push_back(name);
    }

    void Environment(const std::string &what) {
        if (environment_.insert(what).second) report_.environment.push_back(what);
    }
};

std::string Quoted(const std::vector<std::string> &names) {
    std::string out;
    for (size_t n = 0; n < names.size(); ++n) out += (n ? ", '" : "'") + names[n] + "'";
    return out;
}

void Merge(PiNet::Report &into, const PiNet::Report &from) {
    for (auto const &n : from.unbound)
        if (std::find(into.unbound.begin(), into.unbound.end(), n) == into.unbound.end()) into.unbound.push_back(n);
    for (auto const &n : from.environment)
        if (std::find(into.environment.begin(), into.environment.end(), n) == into.environment.end())
            into.environment.push_back(n);
}

}  // namespace

std::string PiNet::Report::ToString() const {
    std::string out;
    if (!unbound.empty())
        out += "uses " + Quoted(unbound) + ", which it does not bind";
    if (!environment.empty()) {
        if (!out.empty()) out += "; ";
        out += "uses " + Quoted(environment) + ", which reach" + (environment.size() == 1 ? "es" : "") +
               " into the sending node";
    }
    return out;
}

PiNet::Report PiNet::Check(const Continuation &cont, const SystemNames &system) {
    Report report;
    Walker(system, report).Walk(cont);
    return report;
}

PiNet::SystemNames PiNet::SystemNamesOf(const Tree *tree) {
    if (tree == nullptr) return {};
    Object root = tree->GetRoot();
    return [root](const Label &name) {
        if (!root.Exists()) return false;
        for (const char *dir : {"Bin", "Sys", "Types"}) {
            if (!root.Has(Label(dir))) continue;
            Object folder = root.Get(Label(dir));
            if (folder.Exists() && folder.Has(name)) return true;
        }
        return false;
    };
}

PiNet::Report PiNet::Check(Object payload, const Tree *tree) {
    Report report;
    if (!payload.Exists()) return report;
    if (payload.IsType<Continuation>()) return Check(ConstDeref<Continuation>(payload), SystemNamesOf(tree));
    if (payload.IsType<Array>()) {
        const Array &items = ConstDeref<Array>(payload);
        for (int i = 0; i < items.Size(); ++i) Merge(report, Check(items.At(i), tree));
        return report;
    }
    if (payload.IsType<BinaryStream>()) {
        // Already frozen: thaw a copy to see what it holds, then rewind it
        // so the send reads it from the start.
        Object thawed = Bin::Thaw(payload);
        Deref<BinaryStream>(payload).Reset();
        if (thawed.Exists() && !thawed.IsType<BinaryStream>()) return Check(thawed, tree);
    }
    return report;
}

void PiNet::Require(Object payload, const Tree *tree) {
    const Report report = Check(payload, tree);
    if (report.Ok()) return;
    const std::string message =
        "PiNet: not transportable: it " + report.ToString() +
        ". Pass values in on the stack, or bind them in the payload with 'name #";
    KAI_THROW_1(Base, message.c_str());
}

KAI_END
