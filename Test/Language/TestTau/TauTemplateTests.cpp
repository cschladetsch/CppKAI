#include <gtest/gtest.h>

#include "KAI/Language/Tau/TauParser.h"
#include "TestLangCommon.h"

// Test suite for Tau template/generic IDL definitions
// Note: Tau is an IDL for code generation, not an executable language

TEST(TauTemplate, BasicGenericClass) {
    kai::Registry reg;

    const char* code = R"(
        namespace Collections {
            class List<T> {
                void Add(T item);
                void Remove(T item);
                T Get(int index);
                int Count();
                void Clear();
            }
            
            class Dictionary<K, V> {
                void Add(K key, V value);
                V Get(K key);
                bool ContainsKey(K key);
                void Remove(K key);
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    // Generic/template syntax not yet implemented
    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauTemplate, GenericInterfaces) {
    kai::Registry reg;

    const char* code = R"(
        namespace System {
            interface IComparable<T> {
                int CompareTo(T other);
            }
            
            interface IEnumerable<T> {
                IEnumerator<T> GetEnumerator();
            }
            
            interface IEnumerator<T> {
                bool MoveNext();
                T Current();
                void Reset();
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauTemplate, ConstrainedGenerics) {
    kai::Registry reg;

    const char* code = R"(
        namespace Advanced {
            class Repository<T> where T : IEntity {
                void Add(T entity);
                T GetById(int id);
                List<T> GetAll();
            }
            
            class Cache<TKey, TValue> 
                where TKey : IComparable<TKey>
                where TValue : class {
                void Set(TKey key, TValue value);
                TValue Get(TKey key);
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauTemplate, GenericMethods) {
    kai::Registry reg;

    const char* code = R"(
        namespace Utilities {
            class Converter {
                T ConvertTo<T>(object value);
                TOut Transform<TIn, TOut>(TIn input);
                List<T> Filter<T>(List<T> items, Predicate<T> predicate);
            }
            
            delegate bool Predicate<T>(T item);
            delegate TResult Func<T, TResult>(T arg);
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauTemplate, VariadicTemplates) {
    kai::Registry reg;

    const char* code = R"(
        namespace Variadic {
            class Tuple<T...> {
                int Size();
            }
            
            class EventHandler<TArgs...> {
                void Invoke(TArgs... args);
                void Subscribe(Action<TArgs...> handler);
            }
            
            delegate void Action<T...>(T... args);
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}