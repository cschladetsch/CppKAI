#pragma once

#include "KAI/Core/Exception/ExceptionBase.h"

KAI_BEGIN

#define KAI_FILE_LOCATION KAI_NAMESPACE(FileLocation)(__FILE__, __LINE__)

#define KAI_FUNCTION_NAME \
    KAI_NAMESPACE(FileLocation)(

// Always define exception macros, regardless of KAI_USE_EXCEPTIONS
// This ensures that all code that uses these macros will compile

#define KAI_RETHROW() throw;

#define KAI_THROW_0(E) throw KAI_NAMESPACE(Exception::E)(KAI_FILE_LOCATION)

#define KAI_THROW_1(E, A0) \
    throw KAI_NAMESPACE(Exception::E)(KAI_FILE_LOCATION, A0)

#define KAI_THROW_2(E, A0, A1) \
    throw KAI_NAMESPACE(Exception::E)(KAI_FILE_LOCATION, A0, A1)

#define KAI_THROW_3(E, A0, A1, A2) \
    throw KAI_NAMESPACE(Exception::E)(KAI_FILE_LOCATION, A0, A1, A2)

#define KAI_NOT_IMPLEMENTED() \
    throw KAI_NAMESPACE(Exception::NotImplemented)(KAI_FILE_LOCATION);

#define KAI_NOT_IMPLEMENTED_1(T) \
    throw KAI_NAMESPACE(Exception::NotImplemented)(KAI_FILE_LOCATION, T);

#define KAI_TRY try

#define KAI_CATCH_RETHROW_ALL()        \
    catch (Exception::Base & E) {      \
        E.AddTrace(KAI_FILE_LOCATION); \
        throw E;                       \
    }

#define KAI_CATCH(TYPE, E) catch (TYPE & E)

#define KAI_CATCH_IGNORE(TYPE) catch (TYPE &)

#define KAI_CATCH_ALL() catch (...)

KAI_END
