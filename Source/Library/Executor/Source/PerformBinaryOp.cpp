#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Exception.h>
#include <KAI/Executor/Executor.h>

KAI_BEGIN

// Enhanced version of PerformBinaryOp that handles all operation types using
// KAI type traits
Object Executor::PerformBinaryOp(Object const &A, Object const &B,
                                 Operation::Type op) {
    // KAI_TRACE() << "PerformBinaryOp called with operation: "
    //             << Operation::ToString(op);
    try {
        // Validate inputs
        if (!A.Valid()) {
            KAI_TRACE_ERROR() << "PerformBinaryOp: First argument is invalid";
            return Object();
        }

        if (!B.Valid()) {
            KAI_TRACE_ERROR() << "PerformBinaryOp: Second argument is invalid";
            return Object();
        }

        // Ensure we have a valid registry to create new objects
        Registry *registry = A.GetRegistry();
        if (!registry) {
            registry = B.GetRegistry();
            if (!registry) {
                // Try to use the executor's registry if available through data
                // stack
                if (data_.Exists() && data_.GetRegistry() != nullptr) {
                    registry = data_.GetRegistry();
                } else {
                    // Try to use Self if available
                    if (Self && Self->GetRegistry()) {
                        registry = Self->GetRegistry();
                    } else {
                        KAI_TRACE_ERROR()
                            << "PerformBinaryOp: No valid registry found";
                        return Object();
                    }
                }
            }
        }

        // Helper function to create a new object, ensuring it has a valid
        // registry
        auto createNew = [registry](auto value) -> Object {
            return registry->New(value);
        };

        using Type::Properties;

        // First, handle the operation based on type using KAI type traits
        switch (op) {
            // Arithmetic operations
            case Operation::Plus: {
                // KAI_TRACE() << "Plus operation";
                // Int + Int = Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result = Type::Traits<int>::Plus::Perform(
                        ConstDeref<int>(A), ConstDeref<int>(B));
                    return createNew(result);
                }
                // Float + Float = Float
                else if (A.IsType<float>() && B.IsType<float>()) {
                    float result = Type::Traits<float>::Plus::Perform(
                        ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Float + Int = Float
                else if (A.IsType<float>() && B.IsType<int>()) {
                    float result = ConstDeref<float>(A) +
                                   static_cast<float>(ConstDeref<int>(B));
                    return createNew(result);
                }
                // Int + Float = Float
                else if (A.IsType<int>() && B.IsType<float>()) {
                    float result = static_cast<float>(ConstDeref<int>(A)) +
                                   ConstDeref<float>(B);
                    return createNew(result);
                }
                // String + String = String (concatenation)
                else if (A.IsType<String>() && B.IsType<String>()) {
                    String result = Type::Traits<String>::Plus::Perform(
                        ConstDeref<String>(A), ConstDeref<String>(B));
                    return createNew(result);
                }
                // Pathname + Pathname = combined pathname
                else if (A.IsType<Pathname>() && B.IsType<Pathname>()) {
                    Pathname result = Type::Traits<Pathname>::Plus::Perform(
                        ConstDeref<Pathname>(A), ConstDeref<Pathname>(B));
                    return createNew(result);
                }
                // Array + Array = concatenated array
                else if (A.IsType<Array>() && B.IsType<Array>()) {
                    // KAI_TRACE() << "Performing Array + Array operation";
                    // Direct implementation since Type::Traits<Array>::Plus
                    // isn't instantiated correctly
                    const Array &arr1 = ConstDeref<Array>(A);
                    const Array &arr2 = ConstDeref<Array>(B);
                    Array result = arr1 + arr2;  // Use our operator+
                    return createNew(result);
                }
                // For other types, use the ClassBase's operation methods
                else if (A.GetTypeNumber() == B.GetTypeNumber() &&
                         A.GetClass() && B.GetClass()) {
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
                    return createNew(result);
                }
                // Float - Float = Float
                else if (A.IsType<float>() && B.IsType<float>()) {
                    float result = Type::Traits<float>::Minus::Perform(
                        ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Float - Int = Float
                else if (A.IsType<float>() && B.IsType<int>()) {
                    float result = ConstDeref<float>(A) -
                                   static_cast<float>(ConstDeref<int>(B));
                    return createNew(result);
                }
                // Int - Float = Float
                else if (A.IsType<int>() && B.IsType<float>()) {
                    float result = static_cast<float>(ConstDeref<int>(A)) -
                                   ConstDeref<float>(B);
                    return createNew(result);
                }
                // For other types, use the ClassBase's operation methods
                else if (A.GetTypeNumber() == B.GetTypeNumber() &&
                         A.GetClass() && B.GetClass()) {
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
                    return createNew(result);
                }
                // Float * Float = Float
                else if (A.IsType<float>() && B.IsType<float>()) {
                    float result = Type::Traits<float>::Multiply::Perform(
                        ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Float * Int = Float
                else if (A.IsType<float>() && B.IsType<int>()) {
                    float result = ConstDeref<float>(A) *
                                   static_cast<float>(ConstDeref<int>(B));
                    return createNew(result);
                }
                // Int * Float = Float
                else if (A.IsType<int>() && B.IsType<float>()) {
                    float result = static_cast<float>(ConstDeref<int>(A)) *
                                   ConstDeref<float>(B);
                    return createNew(result);
                }
                // For other types, use the ClassBase's operation methods
                else if (A.GetTypeNumber() == B.GetTypeNumber() &&
                         A.GetClass() && B.GetClass()) {
                    const ClassBase *classPtr = A.GetClass();
                    if (classPtr->HasOperation(Properties::Multiply)) {
                        try {
                            StorageBase *result = classPtr->Multiply(
                                A.GetStorageBase(), B.GetStorageBase());
                            if (result) {
                                return Object(ObjectConstructParams(result));
                            }
                        } catch (const Exception::Base &e) {
                            KAI_TRACE_ERROR() << "Multiply operation failed: "
                                              << e.ToString();
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
                    return createNew(result);
                }
                // Float / Float = Float
                else if (A.IsType<float>() && B.IsType<float>()) {
                    float divisor = ConstDeref<float>(B);
                    if (divisor == 0.0f) {
                        KAI_THROW_0(DivideByZero);
                    }
                    float result = Type::Traits<float>::Divide::Perform(
                        ConstDeref<float>(A), divisor);
                    return createNew(result);
                }
                // Float / Int = Float
                else if (A.IsType<float>() && B.IsType<int>()) {
                    int divisor = ConstDeref<int>(B);
                    if (divisor == 0) {
                        KAI_THROW_0(DivideByZero);
                    }
                    float result =
                        ConstDeref<float>(A) / static_cast<float>(divisor);
                    return createNew(result);
                }
                // Int / Float = Float
                else if (A.IsType<int>() && B.IsType<float>()) {
                    float divisor = ConstDeref<float>(B);
                    if (divisor == 0.0f) {
                        KAI_THROW_0(DivideByZero);
                    }
                    float result =
                        static_cast<float>(ConstDeref<int>(A)) / divisor;
                    return createNew(result);
                }
                // For other types, use the ClassBase's operation methods
                else if (A.GetTypeNumber() == B.GetTypeNumber() &&
                         A.GetClass() && B.GetClass()) {
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
                    return createNew(result);
                }
                break;

            // Comparison operations
            case Operation::Equiv:
                // Int == Int -> bool
                if (A.IsType<int>() && B.IsType<int>()) {
                    bool result = Type::Traits<int>::Equiv::Perform(
                        ConstDeref<int>(A), ConstDeref<int>(B));
                    return createNew(result);
                }
                // Float == Float -> bool
                else if (A.IsType<float>() && B.IsType<float>()) {
                    bool result = Type::Traits<float>::Equiv::Perform(
                        ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Float == Int -> bool
                else if (A.IsType<float>() && B.IsType<int>()) {
                    bool result = ConstDeref<float>(A) ==
                                  static_cast<float>(ConstDeref<int>(B));
                    return createNew(result);
                }
                // Int == Float -> bool
                else if (A.IsType<int>() && B.IsType<float>()) {
                    bool result = static_cast<float>(ConstDeref<int>(A)) ==
                                  ConstDeref<float>(B);
                    return createNew(result);
                }
                // Bool == Bool -> bool
                else if (A.IsType<bool>() && B.IsType<bool>()) {
                    bool result = Type::Traits<bool>::Equiv::Perform(
                        ConstDeref<bool>(A), ConstDeref<bool>(B));
                    return createNew(result);
                }
                // String == String -> bool
                else if (A.IsType<String>() && B.IsType<String>()) {
                    bool result = Type::Traits<String>::Equiv::Perform(
                        ConstDeref<String>(A), ConstDeref<String>(B));
                    return createNew(result);
                }
                // General object equality
                else {
                    bool result = A == B;
                    return createNew(result);
                }
                break;

            case Operation::NotEquiv:
                // Invert Equiv result
                {
                    Object equivResult =
                        PerformBinaryOp(A, B, Operation::Equiv);
                    if (equivResult.IsType<bool>()) {
                        bool result = !ConstDeref<bool>(equivResult);
                        return createNew(result);
                    }
                }
                break;

            case Operation::Less:
                // Int < Int -> bool
                if (A.IsType<int>() && B.IsType<int>()) {
                    bool result = Type::Traits<int>::Less::Perform(
                        ConstDeref<int>(A), ConstDeref<int>(B));
                    return createNew(result);
                }
                // Float < Float -> bool
                else if (A.IsType<float>() && B.IsType<float>()) {
                    bool result = Type::Traits<float>::Less::Perform(
                        ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Float < Int -> bool
                else if (A.IsType<float>() && B.IsType<int>()) {
                    bool result = ConstDeref<float>(A) <
                                  static_cast<float>(ConstDeref<int>(B));
                    return createNew(result);
                }
                // Int < Float -> bool
                else if (A.IsType<int>() && B.IsType<float>()) {
                    bool result = static_cast<float>(ConstDeref<int>(A)) <
                                  ConstDeref<float>(B);
                    return createNew(result);
                }
                // String < String -> bool
                else if (A.IsType<String>() && B.IsType<String>()) {
                    bool result = Type::Traits<String>::Less::Perform(
                        ConstDeref<String>(A), ConstDeref<String>(B));
                    return createNew(result);
                }
                // For other types, use the ClassBase's operation methods
                else if (A.GetTypeNumber() == B.GetTypeNumber() &&
                         A.GetClass() && B.GetClass()) {
                    const ClassBase *classPtr = A.GetClass();
                    if (classPtr->HasOperation(Properties::Less)) {
                        try {
                            bool result = classPtr->Less(A.GetStorageBase(),
                                                         B.GetStorageBase());
                            return createNew(result);
                        } catch (const Exception::Base &e) {
                            KAI_TRACE_ERROR()
                                << "Less operation failed: " << e.ToString();
                            return createNew(false);
                        }
                    }
                }
                break;

            case Operation::Greater:
                // Int > Int -> bool (invert Less)
                if (A.IsType<int>() && B.IsType<int>()) {
                    bool result = Type::Traits<int>::Less::Perform(
                        ConstDeref<int>(B), ConstDeref<int>(A));
                    return createNew(result);
                }
                // Float > Float -> bool
                else if (A.IsType<float>() && B.IsType<float>()) {
                    bool result = Type::Traits<float>::Less::Perform(
                        ConstDeref<float>(B), ConstDeref<float>(A));
                    return createNew(result);
                }
                // Float > Int -> bool
                else if (A.IsType<float>() && B.IsType<int>()) {
                    bool result = static_cast<float>(ConstDeref<int>(B)) <
                                  ConstDeref<float>(A);
                    return createNew(result);
                }
                // Int > Float -> bool
                else if (A.IsType<int>() && B.IsType<float>()) {
                    bool result = ConstDeref<float>(B) <
                                  static_cast<float>(ConstDeref<int>(A));
                    return createNew(result);
                }
                // String > String -> bool
                else if (A.IsType<String>() && B.IsType<String>()) {
                    bool result = Type::Traits<String>::Less::Perform(
                        ConstDeref<String>(B), ConstDeref<String>(A));
                    return createNew(result);
                }
                // For other types, use the ClassBase's operation methods
                else if (A.GetTypeNumber() == B.GetTypeNumber() &&
                         A.GetClass() && B.GetClass()) {
                    const ClassBase *classPtr = A.GetClass();
                    if (classPtr->HasOperation(Properties::Greater)) {
                        try {
                            bool result = classPtr->Greater(A.GetStorageBase(),
                                                            B.GetStorageBase());
                            return createNew(result);
                        } catch (const Exception::Base &e) {
                            KAI_TRACE_ERROR()
                                << "Greater operation failed: " << e.ToString();
                            return createNew(false);
                        }
                    }
                }
                break;

            case Operation::LessOrEquiv:
                // Check if A is less than B or equivalent to B
                {
                    Object lessResult = PerformBinaryOp(A, B, Operation::Less);
                    Object equivResult =
                        PerformBinaryOp(A, B, Operation::Equiv);

                    if (lessResult.IsType<bool>() &&
                        equivResult.IsType<bool>()) {
                        bool result = ConstDeref<bool>(lessResult) ||
                                      ConstDeref<bool>(equivResult);
                        return createNew(result);
                    }
                }
                break;

            case Operation::GreaterOrEquiv:
                // Check if A is greater than B or equivalent to B
                {
                    Object greaterResult =
                        PerformBinaryOp(A, B, Operation::Greater);
                    Object equivResult =
                        PerformBinaryOp(A, B, Operation::Equiv);

                    if (greaterResult.IsType<bool>() &&
                        equivResult.IsType<bool>()) {
                        bool result = ConstDeref<bool>(greaterResult) ||
                                      ConstDeref<bool>(equivResult);
                        return createNew(result);
                    }
                }
                break;

            // Logical operations
            case Operation::LogicalAnd:
                // Bool && Bool -> Bool
                if (A.IsType<bool>() && B.IsType<bool>()) {
                    bool result = ConstDeref<bool>(A) && ConstDeref<bool>(B);
                    return createNew(result);
                }
                break;

            case Operation::LogicalOr:
                // Bool || Bool -> Bool
                if (A.IsType<bool>() && B.IsType<bool>()) {
                    bool result = ConstDeref<bool>(A) || ConstDeref<bool>(B);
                    return createNew(result);
                }
                break;

            case Operation::LogicalXor:
                // Bool XOR Bool -> Bool
                if (A.IsType<bool>() && B.IsType<bool>()) {
                    bool result = ConstDeref<bool>(A) != ConstDeref<bool>(B);
                    return createNew(result);
                }
                break;

            // Bitwise operations
            case Operation::BitwiseAnd:
                // Int & Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result = ConstDeref<int>(A) & ConstDeref<int>(B);
                    return createNew(result);
                }
                break;

            case Operation::BitwiseOr:
                // Int | Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result = ConstDeref<int>(A) | ConstDeref<int>(B);
                    return createNew(result);
                }
                break;

            case Operation::BitwiseXor:
                // Int ^ Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result = ConstDeref<int>(A) ^ ConstDeref<int>(B);
                    return createNew(result);
                }
                break;

            case Operation::LeftShift:
                // Int << Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result = ConstDeref<int>(A) << ConstDeref<int>(B);
                    return createNew(result);
                }
                break;

            case Operation::RightShift:
                // Int >> Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result = ConstDeref<int>(A) >> ConstDeref<int>(B);
                    return createNew(result);
                }
                break;

            case Operation::Index:
                // Array[Int] -> Object
                if (A.IsType<Array>() && B.IsType<int>()) {
                    try {
                        const Array &array = ConstDeref<Array>(A);
                        int index = ConstDeref<int>(B);

                        if (index >= 0 &&
                            index < static_cast<int>(array.Size())) {
                            // For arrays, we can access elements directly
                            return array.At(index);
                        }

                        KAI_THROW_1(BadIndex, index);
                    } catch (const std::exception &e) {
                        KAI_TRACE_ERROR()
                            << "Exception in Array indexing: " << e.what();
                    }
                }
                break;

            // Min/Max operations
            case Operation::Min:
                // Int min Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result =
                        std::min(ConstDeref<int>(A), ConstDeref<int>(B));
                    return createNew(result);
                }
                // Float min Float -> Float
                else if (A.IsType<float>() && B.IsType<float>()) {
                    float result =
                        std::min(ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Handle mixed int/float types by comparing as floats
                else if ((A.IsType<int>() || A.IsType<float>()) &&
                         (B.IsType<int>() || B.IsType<float>())) {
                    Object less_result = PerformBinaryOp(A, B, Operation::Less);
                    if (less_result.Exists() && less_result.IsType<bool>()) {
                        return ConstDeref<bool>(less_result) ? A : B;
                    }
                }
                break;

            case Operation::Max:
                // Int max Int -> Int
                if (A.IsType<int>() && B.IsType<int>()) {
                    int result =
                        std::max(ConstDeref<int>(A), ConstDeref<int>(B));
                    return createNew(result);
                }
                // Float max Float -> Float
                else if (A.IsType<float>() && B.IsType<float>()) {
                    float result =
                        std::max(ConstDeref<float>(A), ConstDeref<float>(B));
                    return createNew(result);
                }
                // Handle mixed int/float types by comparing as floats
                else if ((A.IsType<int>() || A.IsType<float>()) &&
                         (B.IsType<int>() || B.IsType<float>())) {
                    Object less_result = PerformBinaryOp(B, A, Operation::Less);
                    if (less_result.Exists() && less_result.IsType<bool>()) {
                        return ConstDeref<bool>(less_result) ? A : B;
                    }
                }
                break;

            default:
                // For unsupported operations, provide a helpful error message
                KAI_TRACE_ERROR()
                    << "Unsupported operation in PerformBinaryOp: "
                    << Operation::ToString(op);
                // Fall through to default handling
                break;
        }

        // If we reach here, it means we couldn't handle the operation with the
        // given types
        if (A.Valid() && A.GetClass() && B.Valid() && B.GetClass()) {
            KAI_TRACE_ERROR()
                << "Unsupported types for operation: "
                << A.GetClass()->GetName() << " and " << B.GetClass()->GetName()
                << " for operation " << Operation::ToString(op);
        } else {
            KAI_TRACE_ERROR()
                << "Invalid objects for operation: " << Operation::ToString(op);
        }

        // Return a default value based on operation type and operand types
        // Ensure we use the registry we found earlier

        // Arithmetic operations typically return numeric types
        if (op == Operation::Plus || op == Operation::Minus ||
            op == Operation::Multiply || op == Operation::Divide ||
            op == Operation::Modulo) {
            if (A.IsType<int>()) return createNew(0);
            if (A.IsType<float>()) return createNew(0.0f);
            if (A.IsType<double>()) return createNew(0.0);
        }

        // Comparison operations typically return boolean
        if (op == Operation::Equiv || op == Operation::NotEquiv ||
            op == Operation::Less || op == Operation::Greater ||
            op == Operation::LessOrEquiv || op == Operation::GreaterOrEquiv ||
            op == Operation::LogicalAnd || op == Operation::LogicalOr ||
            op == Operation::LogicalXor) {
            return createNew(false);
        }

        // String operations
        if (A.IsType<String>()) {
            return createNew(String(""));
        }

        // If we still can't determine a suitable return type, return A if
        // valid, otherwise an empty object
        return A.Valid() ? A : Object();
    } catch (const Exception::Base &e) {
        KAI_TRACE_ERROR() << "PerformBinaryOp: KAI exception: " << e.ToString();
        return Object();
    } catch (const std::exception &e) {
        KAI_TRACE_ERROR() << "PerformBinaryOp: std::exception: " << e.what();
        return Object();
    } catch (...) {
        KAI_TRACE_ERROR() << "PerformBinaryOp: Unknown exception";
        return Object();
    }
}

KAI_END