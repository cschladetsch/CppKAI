#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Core/Pathname.h>

KAI_BEGIN

class Tree {
   public:
    typedef std::list<Object> SearchPath;

   private:
    SearchPath path_;
    Object root_, scope_;
    Pathname current_;

   public:
    void SetRoot(const Object &Q) { root_ = Q; }
    void AddSearchPath(const Pathname &);
    void AddSearchPath(const Object &);

    Object Resolve(const Pathname &) const;
    Object Resolve(const Label &) const;

    Object GetRoot() const { return root_; }
    Object GetScope() const { return scope_; }
    const SearchPath &GetSearchPath() const { return path_; }

    void SetScope(const Object &);
    void SetScope(const Pathname &);

    // void SetSearchPath(const SearchPath &);
    // void GetChildren() const;
};

KAI_END
