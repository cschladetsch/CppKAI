#include <KAI/Core/BuiltinTypes.h>

KAI_BEGIN

namespace Exception {
std::string Base::ToString() const {
    StringStream S;
    std::string loc = location.ToString().c_str();
    loc = loc.substr(loc.find_last_of('/') + 1);
    S << loc << text << ": ";
    WriteExtendedInformation(S);
    return S.ToString().c_str();
}

void TypeMismatch::WriteExtendedInformation(StringStream &S) const {
    S << "expected=" << Type::Number(first).ToString()
      << ", got=" << Type::Number(second).ToString();
}

void UnknownTypeNumber::WriteExtendedInformation(StringStream &S) const {
    S << "type=" << type_number;
}

void UnknownObject::WriteExtendedInformation(StringStream &S) const {
    S << "handle=" << (int)handle.GetValue();
}

void ObjectNotFound::WriteExtendedInformation(StringStream &S) const {
    S << "label=" << label;
}

void NoOperation::WriteExtendedInformation(StringStream &S) const {
    S << "type=" << type_number << ", type_property=" << type_property;
}

void PacketExtraction::WriteExtendedInformation(StringStream &S) const {
    S << "type=" << type_number;
}

void OutOfBounds::WriteExtendedInformation(StringStream &S) const {
    S << "type=" << type_number << ", index=" << index;
}

void PacketInsertion::WriteExtendedInformation(StringStream &S) const {
    S << "type=" << type_number;
}

void CannotResolve::WriteExtendedInformation(StringStream &S) const {
    if (!path.Empty())
        S << "path=" << path;
    else if (!label.Empty())
        S << "label=" << label;
    else
        S << "object=" << object;
}

void InvalidPathname::WriteExtendedInformation(StringStream &S) const {
    S << "text=" << text;
}

void ObjectNotInTree::WriteExtendedInformation(StringStream &S) const {
    S << "object=" << object;
}

void UnknownMethod::WriteExtendedInformation(StringStream &S) const {
    S << "name_=" << name_ << ", class=" << class_name_;
}

void UnknownHandle::WriteExtendedInformation(StringStream &S) const {
    S << "handle=" << handle.GetValue();
}

void InvalidStringLiteral::WriteExtendedInformation(StringStream &S) const {
    S << "text='" << text << "'";
}

void CannotNew::WriteExtendedInformation(StringStream &S) const {
    S << "object=" << arg;
}

void UnknownKey::WriteExtendedInformation(StringStream &S) const {
    S << "key=" << key;
}

void NotImplemented::WriteExtendedInformation(StringStream &S) const {
    S << "what=" << text;
}

void BadIndex::WriteExtendedInformation(StringStream &S) const {
    S << "index=" << index;
}

void InvalidIdentifier::WriteExtendedInformation(StringStream &S) const {
    S << "what='" << what << "'";
}

void FileNotFound::WriteExtendedInformation(StringStream &S) const {
    S << "filename='" << filename << "'";
}
void UnknownProperty::WriteExtendedInformation(StringStream &S) const {
    S << "class=" << klass << ", property=" << prop;
}
}  // namespace Exception

StringStream &operator<<(StringStream &S, Exception::Base const &E) {
    return S << E.ToString();
}

KAI_END

// EOF
