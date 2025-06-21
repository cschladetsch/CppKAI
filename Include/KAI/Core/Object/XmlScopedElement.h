
#pragma once

#include <KAI/Core/Config/Base.h>

KAI_BEGIN

/// Writes a <Element name1="text1" ... nameN="textN">optional body</Element>
class XmlScopedElement {
    typedef String String;
    typedef StringStream StringStream;

    StringStream &xml;
    String name_;

   public:
    XmlScopedElement(StringStream &X, const char *N) : xml(X), name_(N) {
        xml << "<" << name_ << ">";
    }

    /// A Property of an Xml Element is a name/value pair
    struct Property {
        String name_;
        String text;
        Property(const char *N, const char *T) : name_(N), text(T) {}
        void Write(StringStream &str) {
            str << " " << name_ << "=\"" << text << "\" ";
        }
    };

    XmlScopedElement(StringStream &X, const char *N, const char *id,
                     const char *text)
        : xml(X), name_(N) {
        xml << "<" << name_;
        Property(id, text).Write(xml);
        xml << ">";
    }

    template <class II>
    XmlScopedElement(StringStream &X, const char *N, II prop, II last)
        : xml(X), name_(N) {
        xml << "<" << name_ << " ";
        for (; prop != last; ++prop) prop->Write(xml);
        xml << ">";
    }

    ~XmlScopedElement() { xml << "</" << name_ << ">"; }

   private:
    XmlScopedElement &operator=(const XmlScopedElement &);
};

KAI_END
