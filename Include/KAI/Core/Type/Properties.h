#pragma once

#include <KAI/Core/Config/Base.h>

KAI_TYPE_BEGIN

// Describes the properties_ of a promoted type
struct Properties {
    enum Type : int {
        Nothing = 0,
        Assign = 1 << 0,
        Reflected = 1 << 1,
        Plus = 1 << 2,
        Minus = 1 << 3,
        Arithmetic = Plus | Minus,
        Less = 1 << 6,
        Equiv = 1 << 7,
        Greater = 1 << 8,
        Relational = Less | Equiv | Greater,
        StringStreamInsert = 1 << 9,
        StringStreamExtract = 1 << 10,
        BinaryStreamInsert = 1 << 11,
        BinaryStreamExtract = 1 << 12,
        StringStreaming = StringStreamInsert | StringStreamExtract,
        BinaryStreaming = BinaryStreamInsert | BinaryStreamExtract,
        Streaming = StringStreaming | BinaryStreaming,
        NoHashValue = 1 << 13,
        CalcHashValue = 1 << 14,
        Container = 1 << 15,
        Process = 1 << 16,
        Absolute = 1 << 17,
        Divide = 1 << 18,
        Multiply = 1 << 19,
        Multiplicative = Divide | Multiply,
        XmlOutput = 1 << 20,
        StreamExtract = 1 << 21,
        Boolean = 1 << 22,
    };
};

KAI_TYPE_END
