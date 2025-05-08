#pragma once

#include <KAI/Core/Base.h>
#include <KAI/Core/Object/Label.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Core/StringStream.h>
#include <KAI/Core/Type/Number.h>

// TODO: remove this and have DotGraph use Pimpl
// not very important as few things include this file.
#include <set>

KAI_BEGIN

// Generates GraphViz script showing the depedancy graph given an object _root
struct DotGraph : StringStream {
   private:
    std::set<Label> excluded_names;
    std::set<Type::Number> excluded_types;

   public:
    DotGraph();
    DotGraph(Object, String const &filename);

    void Generate(Object, String const &filename);

    void ExcludeLabel(Label const &);
    template <class T>
    void ExcludeType() {
        excluded_types.insert(Type::Traits<T>::Number);
    }
    void WriteHeader();
    void WriteFooter();
    void WriteToFile(const char *);

    void Add(Object);
    String GetText();

    friend DotGraph &operator<<(DotGraph &graph, Object const &object);

   protected:
    bool IsExcluded(Object const &) const;
};

DotGraph &operator<<(DotGraph &graph, Object const &object);

KAI_END
