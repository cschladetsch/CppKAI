#include <KAI/Core/DotGraph.h>

#include <fstream>

#include "KAI/Core/BuiltinTypes.h"

using namespace std;
using namespace kai;

KAI_BEGIN

DotGraph::DotGraph() {}

DotGraph::DotGraph(Object root, String const &filename) {
    Generate(root, filename);
}

void DotGraph::Generate(Object root, String const &filename) {
    Clear();
    WriteHeader();
    *this << root;
    WriteFooter();
    WriteToFile(filename.c_str());
}

void DotGraph::WriteHeader() { *this << "digraph G\n{\n"; }

void DotGraph::WriteFooter() { *this << "}\n"; }

void DotGraph::ExcludeLabel(Label const &label) {
    excluded_names.insert(label);
}

String DotGraph::GetText() { return ToString(); }

DotGraph &operator<<(DotGraph &graph, Object const &object) {
    bool excluded = !object.Exists() || graph.IsExcluded(object);
    if (excluded) return graph;

    int object_id = object.GetHandle().GetValue();
    StringStream label;
    label << object.GetLabel().ToString() << "[" << object.GetClass()->GetName()
          << "]";

    graph << object_id << " [shape=box label=\"" << label.ToString().c_str()
          << "\"";
    graph << " color=";
    switch (object.GetColor()) {
        case ObjectColor::White:
            graph << "grey";
            break;
        case ObjectColor::Grey:
            graph << "green";
            break;
        case ObjectColor::Black:
            graph << "black";
            break;
    }

    graph << "]\n";

    if (object.IsType<Continuation>()) {
        return graph;
    }

    Object::ObjectList props, children, contained;
    object.GetPropertyObjects(props);
    object.GetContainedObjects(contained);
    object.GetChildObjects(children);

    for (auto const &child : children) {
        graph << object_id << " -> " << child.GetHandle().GetValue()
              << "[weight=10]\n";
        graph << child;
    }

    for (auto const &prop : props) {
        graph << object_id << " -> " << prop.GetHandle().GetValue()
              << "[style=dashed]\n";
        graph << prop;
    }

    for (auto const &cont : contained) {
        graph << object_id << " -> " << cont.GetHandle().GetValue()
              << "[style=dotted]\n";
        graph << cont;
    }

    return graph;
}

bool DotGraph::IsExcluded(Object const &object) const {
    return false;
#if 0
    if (!object.Exists()) return true;
    return excluded_types.find(object.GetTypeNumber()) !=
               excluded_types.end() ||
           excluded_names.find(object.GetLabel()) != excluded_names.end();
#endif
}

void DotGraph::WriteToFile(const char *filename) {
    String text = GetText();
    std::fstream(filename, std::ios::out).write(text.c_str(), text.size());
}

KAI_END
