#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Exception.h>
#include <KAI/Executor/BinaryOperationHandler.h>

KAI_BEGIN

bool BinaryOperationHandler::IsBinaryOp(Operation::Type op) {
    switch (op) {
        case Operation::Plus:
        case Operation::Minus:
        case Operation::Multiply:
        case Operation::Divide:
        case Operation::Modulo:
        case Operation::Min:
        case Operation::Max:
        case Operation::Equiv:
        case Operation::NotEquiv:
        case Operation::Less:
        case Operation::Greater:
        case Operation::LessOrEquiv:
        case Operation::GreaterOrEquiv:
        case Operation::LogicalAnd:
        case Operation::LogicalOr:
        case Operation::LogicalXor:
        case Operation::BitwiseAnd:
        case Operation::BitwiseOr:
        case Operation::BitwiseXor:
        case Operation::LeftShift:
        case Operation::RightShift:
        case Operation::Index:
            return true;
        default:
            return false;
    }
}

Registry *BinaryOperationHandler::FindRegistry(Object const &A,
                                               Object const &B) {
    Registry *registry = A.GetRegistry();
    if (!registry) {
        registry = B.GetRegistry();
    }
    return registry;
}

template <typename T>
Object BinaryOperationHandler::CreateNew(Registry *registry, T value) {
    if (!registry) {
        KAI_THROW_0(NullObject);
    }
    return registry->New(value);
}

// Explicit instantiations for all types we use
template Object BinaryOperationHandler::CreateNew<int>(Registry *, int);
template Object BinaryOperationHandler::CreateNew<float>(Registry *, float);
template Object BinaryOperationHandler::CreateNew<double>(Registry *, double);
template Object BinaryOperationHandler::CreateNew<bool>(Registry *, bool);
template Object BinaryOperationHandler::CreateNew<String>(Registry *, String);
template Object BinaryOperationHandler::CreateNew<Pathname>(Registry *,
                                                            Pathname);
template Object BinaryOperationHandler::CreateNew<Array>(Registry *, Array);

Object BinaryOperationHandler::Perform(Object const &A, Object const &B,
                                       Operation::Type op) {
    try {
        // Validate inputs
        if (!A.Valid()) {
            KAI_TRACE_ERROR()
                << "BinaryOperationHandler: First argument is invalid";
            return Object();
        }

        if (!B.Valid()) {
            KAI_TRACE_ERROR()
                << "BinaryOperationHandler: Second argument is invalid";
            return Object();
        }

        // Find a valid registry
        Registry *registry = FindRegistry(A, B);
        if (!registry) {
            KAI_TRACE_ERROR()
                << "BinaryOperationHandler: No valid registry found";
            return Object();
        }

        // Dispatch to appropriate handler based on operation type
        switch (op) {
            case Operation::Plus:
            case Operation::Minus:
            case Operation::Multiply:
            case Operation::Divide:
            case Operation::Modulo:
                return PerformArithmetic(A, B, op, registry);

            case Operation::Equiv:
            case Operation::NotEquiv:
            case Operation::Less:
            case Operation::Greater:
            case Operation::LessOrEquiv:
            case Operation::GreaterOrEquiv:
                return PerformComparison(A, B, op, registry);

            case Operation::LogicalAnd:
            case Operation::LogicalOr:
            case Operation::LogicalXor:
                return PerformLogical(A, B, op, registry);

            case Operation::BitwiseAnd:
            case Operation::BitwiseOr:
            case Operation::BitwiseXor:
            case Operation::LeftShift:
            case Operation::RightShift:
                return PerformBitwise(A, B, op, registry);

            case Operation::Min:
            case Operation::Max:
                return PerformMinMax(A, B, op, registry);

            case Operation::Index:
                return PerformIndex(A, B, registry);

            default:
                KAI_TRACE_ERROR()
                    << "Unsupported operation: " << Operation::ToString(op);
                return Object();
        }
    } catch (const Exception::Base &e) {
        KAI_TRACE_ERROR() << "BinaryOperationHandler: KAI exception: "
                          << e.ToString();
        return Object();
    } catch (const std::exception &e) {
        KAI_TRACE_ERROR() << "BinaryOperationHandler: std::exception: "
                          << e.what();
        return Object();
    } catch (...) {
        KAI_TRACE_ERROR() << "BinaryOperationHandler: Unknown exception";
        return Object();
    }
}

Object BinaryOperationHandler::PerformArithmetic(Object const &A,
                                                 Object const &B,
                                                 Operation::Type op,
                                                 Registry *registry) {
    using Type::Properties;

    switch (op) {
        case Operation::Plus: {
            // Int + Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = Type::Traits<int>::Plus::Perform(
                    ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float + Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result = Type::Traits<float>::Plus::Perform(
                    ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Float + Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                float result = ConstDeref<float>(A) +
                               static_cast<float>(ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Int + Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float result = static_cast<float>(ConstDeref<int>(A)) +
                               ConstDeref<float>(B);
                return CreateNew(registry, result);
            }
            // String + String = String (concatenation)
            else if (A.IsType<String>() && B.IsType<String>()) {
                String result = Type::Traits<String>::Plus::Perform(
                    ConstDeref<String>(A), ConstDeref<String>(B));
                return CreateNew(registry, result);
            }
            // Pathname + Pathname = combined pathname
            else if (A.IsType<Pathname>() && B.IsType<Pathname>()) {
                Pathname result = Type::Traits<Pathname>::Plus::Perform(
                    ConstDeref<Pathname>(A), ConstDeref<Pathname>(B));
                return CreateNew(registry, result);
            }
            // Array + Array = concatenated array
            else if (A.IsType<Array>() && B.IsType<Array>()) {
                const Array &arr1 = ConstDeref<Array>(A);
                const Array &arr2 = ConstDeref<Array>(B);
                Array result = arr1 + arr2;
                return CreateNew(registry, result);
            }
            // For other types, use the ClassBase's operation methods
            else if (A.GetTypeNumber() == B.GetTypeNumber() && A.GetClass() &&
                     B.GetClass()) {
                const ClassBase *classPtr = A.GetClass();
                if (classPtr->HasOperation(Properties::Plus)) {
                    try {
                        StorageBase *result = classPtr->Plus(
                            A.GetStorageBase(), B.GetStorageBase());
                        if (result) {
                            return Object(ObjectConstructParams(result));
                        }
                    } catch (const Exception::Base &e) {
                        KAI_TRACE_ERROR()
                            << "Plus operation failed: " << e.ToString();
                    }
                }
            }
            break;
        }

        case Operation::Minus:
            // Int - Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = Type::Traits<int>::Minus::Perform(
                    ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float - Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result = Type::Traits<float>::Minus::Perform(
                    ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Float - Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                float result = ConstDeref<float>(A) -
                               static_cast<float>(ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Int - Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float result = static_cast<float>(ConstDeref<int>(A)) -
                               ConstDeref<float>(B);
                return CreateNew(registry, result);
            }
            // For other types, use the ClassBase's operation methods
            else if (A.GetTypeNumber() == B.GetTypeNumber() && A.GetClass() &&
                     B.GetClass()) {
                const ClassBase *classPtr = A.GetClass();
                if (classPtr->HasOperation(Properties::Minus)) {
                    try {
                        StorageBase *result = classPtr->Minus(
                            A.GetStorageBase(), B.GetStorageBase());
                        if (result) {
                            return Object(ObjectConstructParams(result));
                        }
                    } catch (const Exception::Base &e) {
                        KAI_TRACE_ERROR()
                            << "Minus operation failed: " << e.ToString();
                    }
                }
            }
            break;

        case Operation::Multiply:
            // Int * Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = Type::Traits<int>::Multiply::Perform(
                    ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float * Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result = Type::Traits<float>::Multiply::Perform(
                    ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Float * Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                float result = ConstDeref<float>(A) *
                               static_cast<float>(ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Int * Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float result = static_cast<float>(ConstDeref<int>(A)) *
                               ConstDeref<float>(B);
                return CreateNew(registry, result);
            }
            // For other types, use the ClassBase's operation methods
            else if (A.GetTypeNumber() == B.GetTypeNumber() && A.GetClass() &&
                     B.GetClass()) {
                const ClassBase *classPtr = A.GetClass();
                if (classPtr->HasOperation(Properties::Multiply)) {
                    try {
                        StorageBase *result = classPtr->Multiply(
                            A.GetStorageBase(), B.GetStorageBase());
                        if (result) {
                            return Object(ObjectConstructParams(result));
                        }
                    } catch (const Exception::Base &e) {
                        KAI_TRACE_ERROR()
                            << "Multiply operation failed: " << e.ToString();
                    }
                }
            }
            break;

        case Operation::Divide:
            // Int / Int = Int (integer division)
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_0(DivideByZero);
                }
                int result = Type::Traits<int>::Divide::Perform(
                    ConstDeref<int>(A), divisor);
                return CreateNew(registry, result);
            }
            // Float / Float = Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float divisor = ConstDeref<float>(B);
                if (divisor == 0.0f) {
                    KAI_THROW_0(DivideByZero);
                }
                float result = Type::Traits<float>::Divide::Perform(
                    ConstDeref<float>(A), divisor);
                return CreateNew(registry, result);
            }
            // Float / Int = Float
            else if (A.IsType<float>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_0(DivideByZero);
                }
                float result =
                    ConstDeref<float>(A) / static_cast<float>(divisor);
                return CreateNew(registry, result);
            }
            // Int / Float = Float
            else if (A.IsType<int>() && B.IsType<float>()) {
                float divisor = ConstDeref<float>(B);
                if (divisor == 0.0f) {
                    KAI_THROW_0(DivideByZero);
                }
                float result = static_cast<float>(ConstDeref<int>(A)) / divisor;
                return CreateNew(registry, result);
            }
            // For other types, use the ClassBase's operation methods
            else if (A.GetTypeNumber() == B.GetTypeNumber() && A.GetClass() &&
                     B.GetClass()) {
                const ClassBase *classPtr = A.GetClass();
                if (classPtr->HasOperation(Properties::Divide)) {
                    try {
                        StorageBase *result = classPtr->Divide(
                            A.GetStorageBase(), B.GetStorageBase());
                        if (result) {
                            return Object(ObjectConstructParams(result));
                        }
                    } catch (const Exception::Base &e) {
                        KAI_TRACE_ERROR()
                            << "Divide operation failed: " << e.ToString();
                    }
                }
            }
            break;

        case Operation::Modulo:
            // Int % Int = Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_0(DivideByZero);
                }
                int result = ConstDeref<int>(A) % divisor;
                return CreateNew(registry, result);
            }
            break;
    }

    // If we couldn't handle the operation, return appropriate default
    if (A.IsType<int>()) return CreateNew(registry, 0);
    if (A.IsType<float>()) return CreateNew(registry, 0.0f);
    if (A.IsType<double>()) return CreateNew(registry, 0.0);
    if (A.IsType<String>()) return CreateNew(registry, String(""));

    return A.Valid() ? A : Object();
}

Object BinaryOperationHandler::PerformComparison(Object const &A,
                                                 Object const &B,
                                                 Operation::Type op,
                                                 Registry *registry) {
    switch (op) {
        case Operation::Equiv:
            // Int == Int -> bool
            if (A.IsType<int>() && B.IsType<int>()) {
                bool result = Type::Traits<int>::Equiv::Perform(
                    ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float == Float -> bool
            else if (A.IsType<float>() && B.IsType<float>()) {
                bool result = Type::Traits<float>::Equiv::Perform(
                    ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Float == Int -> bool
            else if (A.IsType<float>() && B.IsType<int>()) {
                bool result = ConstDeref<float>(A) ==
                              static_cast<float>(ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Int == Float -> bool
            else if (A.IsType<int>() && B.IsType<float>()) {
                bool result = static_cast<float>(ConstDeref<int>(A)) ==
                              ConstDeref<float>(B);
                return CreateNew(registry, result);
            }
            // Bool == Bool -> bool
            else if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = Type::Traits<bool>::Equiv::Perform(
                    ConstDeref<bool>(A), ConstDeref<bool>(B));
                return CreateNew(registry, result);
            }
            // String == String -> bool
            else if (A.IsType<String>() && B.IsType<String>()) {
                bool result = Type::Traits<String>::Equiv::Perform(
                    ConstDeref<String>(A), ConstDeref<String>(B));
                return CreateNew(registry, result);
            }
            // General object equality
            else {
                bool result = A == B;
                return CreateNew(registry, result);
            }
            break;

        case Operation::NotEquiv:
            // Invert Equiv result
            {
                Object equivResult = Perform(A, B, Operation::Equiv);
                if (equivResult.IsType<bool>()) {
                    bool result = !ConstDeref<bool>(equivResult);
                    return CreateNew(registry, result);
                }
            }
            break;

        case Operation::Less:
            // Int < Int -> bool
            if (A.IsType<int>() && B.IsType<int>()) {
                bool result = Type::Traits<int>::Less::Perform(
                    ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float < Float -> bool
            else if (A.IsType<float>() && B.IsType<float>()) {
                bool result = Type::Traits<float>::Less::Perform(
                    ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Float < Int -> bool
            else if (A.IsType<float>() && B.IsType<int>()) {
                bool result = ConstDeref<float>(A) <
                              static_cast<float>(ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Int < Float -> bool
            else if (A.IsType<int>() && B.IsType<float>()) {
                bool result = static_cast<float>(ConstDeref<int>(A)) <
                              ConstDeref<float>(B);
                return CreateNew(registry, result);
            }
            // String < String -> bool
            else if (A.IsType<String>() && B.IsType<String>()) {
                bool result = Type::Traits<String>::Less::Perform(
                    ConstDeref<String>(A), ConstDeref<String>(B));
                return CreateNew(registry, result);
            }
            // For other types, use the ClassBase's operation methods
            else if (A.GetTypeNumber() == B.GetTypeNumber() && A.GetClass() &&
                     B.GetClass()) {
                const ClassBase *classPtr = A.GetClass();
                if (classPtr->HasOperation(Type::Properties::Less)) {
                    try {
                        bool result = classPtr->Less(A.GetStorageBase(),
                                                     B.GetStorageBase());
                        return CreateNew(registry, result);
                    } catch (const Exception::Base &e) {
                        KAI_TRACE_ERROR()
                            << "Less operation failed: " << e.ToString();
                        return CreateNew(registry, false);
                    }
                }
            }
            break;

        case Operation::Greater:
            // Int > Int -> bool (invert Less)
            if (A.IsType<int>() && B.IsType<int>()) {
                bool result = Type::Traits<int>::Less::Perform(
                    ConstDeref<int>(B), ConstDeref<int>(A));
                return CreateNew(registry, result);
            }
            // Float > Float -> bool
            else if (A.IsType<float>() && B.IsType<float>()) {
                bool result = Type::Traits<float>::Less::Perform(
                    ConstDeref<float>(B), ConstDeref<float>(A));
                return CreateNew(registry, result);
            }
            // Float > Int -> bool
            else if (A.IsType<float>() && B.IsType<int>()) {
                bool result = static_cast<float>(ConstDeref<int>(B)) <
                              ConstDeref<float>(A);
                return CreateNew(registry, result);
            }
            // Int > Float -> bool
            else if (A.IsType<int>() && B.IsType<float>()) {
                bool result = ConstDeref<float>(B) <
                              static_cast<float>(ConstDeref<int>(A));
                return CreateNew(registry, result);
            }
            // String > String -> bool
            else if (A.IsType<String>() && B.IsType<String>()) {
                bool result = Type::Traits<String>::Less::Perform(
                    ConstDeref<String>(B), ConstDeref<String>(A));
                return CreateNew(registry, result);
            }
            // For other types, use the ClassBase's operation methods
            else if (A.GetTypeNumber() == B.GetTypeNumber() && A.GetClass() &&
                     B.GetClass()) {
                const ClassBase *classPtr = A.GetClass();
                if (classPtr->HasOperation(Type::Properties::Greater)) {
                    try {
                        bool result = classPtr->Greater(A.GetStorageBase(),
                                                        B.GetStorageBase());
                        return CreateNew(registry, result);
                    } catch (const Exception::Base &e) {
                        KAI_TRACE_ERROR()
                            << "Greater operation failed: " << e.ToString();
                        return CreateNew(registry, false);
                    }
                }
            }
            break;

        case Operation::LessOrEquiv:
            // Check if A is less than B or equivalent to B
            {
                Object lessResult = Perform(A, B, Operation::Less);
                Object equivResult = Perform(A, B, Operation::Equiv);

                if (lessResult.IsType<bool>() && equivResult.IsType<bool>()) {
                    bool result = ConstDeref<bool>(lessResult) ||
                                  ConstDeref<bool>(equivResult);
                    return CreateNew(registry, result);
                }
            }
            break;

        case Operation::GreaterOrEquiv:
            // Check if A is greater than B or equivalent to B
            {
                Object greaterResult = Perform(A, B, Operation::Greater);
                Object equivResult = Perform(A, B, Operation::Equiv);

                if (greaterResult.IsType<bool>() &&
                    equivResult.IsType<bool>()) {
                    bool result = ConstDeref<bool>(greaterResult) ||
                                  ConstDeref<bool>(equivResult);
                    return CreateNew(registry, result);
                }
            }
            break;
    }

    // Default for comparison operations is false
    return CreateNew(registry, false);
}

Object BinaryOperationHandler::PerformLogical(Object const &A, Object const &B,
                                              Operation::Type op,
                                              Registry *registry) {
    switch (op) {
        case Operation::LogicalAnd:
            // Bool && Bool -> Bool
            if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = ConstDeref<bool>(A) && ConstDeref<bool>(B);
                return CreateNew(registry, result);
            }
            break;

        case Operation::LogicalOr:
            // Bool || Bool -> Bool
            if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = ConstDeref<bool>(A) || ConstDeref<bool>(B);
                return CreateNew(registry, result);
            }
            break;

        case Operation::LogicalXor:
            // Bool XOR Bool -> Bool
            if (A.IsType<bool>() && B.IsType<bool>()) {
                bool result = ConstDeref<bool>(A) != ConstDeref<bool>(B);
                return CreateNew(registry, result);
            }
            break;
    }

    // Default for logical operations is false
    return CreateNew(registry, false);
}

Object BinaryOperationHandler::PerformBitwise(Object const &A, Object const &B,
                                              Operation::Type op,
                                              Registry *registry) {
    switch (op) {
        case Operation::BitwiseAnd:
            // Int & Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) & ConstDeref<int>(B);
                return CreateNew(registry, result);
            }
            break;

        case Operation::BitwiseOr:
            // Int | Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) | ConstDeref<int>(B);
                return CreateNew(registry, result);
            }
            break;

        case Operation::BitwiseXor:
            // Int ^ Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) ^ ConstDeref<int>(B);
                return CreateNew(registry, result);
            }
            break;

        case Operation::LeftShift:
            // Int << Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) << ConstDeref<int>(B);
                return CreateNew(registry, result);
            }
            break;

        case Operation::RightShift:
            // Int >> Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) >> ConstDeref<int>(B);
                return CreateNew(registry, result);
            }
            break;
    }

    // Default for bitwise operations is 0
    return CreateNew(registry, 0);
}

Object BinaryOperationHandler::PerformMinMax(Object const &A, Object const &B,
                                             Operation::Type op,
                                             Registry *registry) {
    switch (op) {
        case Operation::Min:
            // Int min Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = std::min(ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float min Float -> Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result =
                    std::min(ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Handle mixed int/float types by comparing as floats
            else if ((A.IsType<int>() || A.IsType<float>()) &&
                     (B.IsType<int>() || B.IsType<float>())) {
                Object less_result = Perform(A, B, Operation::Less);
                if (less_result.Exists() && less_result.IsType<bool>()) {
                    return ConstDeref<bool>(less_result) ? A : B;
                }
            }
            break;

        case Operation::Max:
            // Int max Int -> Int
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = std::max(ConstDeref<int>(A), ConstDeref<int>(B));
                return CreateNew(registry, result);
            }
            // Float max Float -> Float
            else if (A.IsType<float>() && B.IsType<float>()) {
                float result =
                    std::max(ConstDeref<float>(A), ConstDeref<float>(B));
                return CreateNew(registry, result);
            }
            // Handle mixed int/float types by comparing as floats
            else if ((A.IsType<int>() || A.IsType<float>()) &&
                     (B.IsType<int>() || B.IsType<float>())) {
                Object less_result = Perform(B, A, Operation::Less);
                if (less_result.Exists() && less_result.IsType<bool>()) {
                    return ConstDeref<bool>(less_result) ? A : B;
                }
            }
            break;
    }

    // Return the first operand as default
    return A.Valid() ? A : Object();
}

Object BinaryOperationHandler::PerformIndex(Object const &A, Object const &B,
                                            Registry *registry) {
    // Array[Int] -> Object
    if (A.IsType<Array>() && B.IsType<int>()) {
        try {
            const Array &array = ConstDeref<Array>(A);
            int index = ConstDeref<int>(B);

            if (index >= 0 && index < static_cast<int>(array.Size())) {
                // For arrays, we can access elements directly
                return array.At(index);
            }

            KAI_THROW_1(BadIndex, index);
        } catch (const std::exception &e) {
            KAI_TRACE_ERROR() << "Exception in Array indexing: " << e.what();
        }
    }

    return Object();
}

KAI_END