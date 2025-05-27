#include <KAI/Core/BuiltinTypes/Dictionary.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/PropertyBase.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Object/Reflected.h"
#include "KAI/Core/Value.h"
#include "KAI/Executor/Continuation.h"
#include "KAI/Executor/Operation.h"

KAI_BEGIN

std::string ToLower(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    std::ranges::transform(text, std::back_inserter(out),
                           [](unsigned char c) { return std::tolower(c); });
    return out;
}

StringStream &operator<<(StringStream &S, ObjectColor::Color C) {
    switch (C) {
        case ObjectColor::White:
            return S << "White";

        case ObjectColor::Grey:
            return S << "Grey";

        case ObjectColor::Black:
            return S << "Black";
    }

    return S << "UnknownColor";
}

void ToStringStream(const Object &Q, StringStream &S, int level) {
    if (!Q.Valid()) {
        S << "[Invalid]\n";
        return;
    }

    const StorageBase &base = Q.GetStorageBase();

    if (Q.GetClass()->HasTraitsProperty(Type::Properties::StringStreamInsert))
        Q.GetClass()->Insert(S, base);
}

// Create indentation string using C++23 views::repeat
std::string CreateIndent(int level) {
    auto spaces = std::views::repeat(' ', 2 * level);
    return std::string(spaces.begin(), spaces.end());
}

void ToXmlStream(const Object &Q, StringStream &S, int level) {
    StringStream indent;
    indent.Append(std::string_view(CreateIndent(level)));

    if (!Q.Valid()) return;

    StorageBase const &base = Q.GetStorageBase();
    ClassBase const &klass = *Q.GetClass();
    S << indent.ToString() << "<Object type='" << klass.GetName() << "' name='"
      << base.GetLabel().ToString()
      //<< "' handle='" << (int)Q.GetHandle().GetValue()
      << "'>\n";

    if (Q.GetClass()->HasTraitsProperty(Type::Properties::StringStreamInsert)) {
        S << indent.ToString() << "  <Value>";
        if (klass.HasOperation(Type::Properties::StringStreamInsert))
            klass.Insert(S, base);
        S << "</Value>\n";
    }

    // Using C++23 views::enumerate to keep track of property index
    const auto &properties = klass.GetProperties();
    for (const auto &[index, prop_pair] : std::views::enumerate(properties)) {
        PropertyBase const &property = *prop_pair.second;
        S << indent.ToString() << "<Property index='" << static_cast<int>(index)
          << "' name='" << property.GetFieldName() << "'>";
        // ToXmlStream(child.second, S, level + 1);
        S << property.GetValue(base);
        S << "</Property>\n";
    }

    const Dictionary &dict = base.GetDictionary();
    for (auto const &child : dict) ToXmlStream(child.second, S, level + 1);

    S << indent.ToString() << "</Object>\n";

    return;
}

KAI_END
