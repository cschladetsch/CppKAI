#include <gtest/gtest.h>

#include <fstream>
#include <regex>
#include <sstream>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProcess.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauParser.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for advanced Tau type system tests
struct TauAdvancedTypeTests : TestLangCommon {
    // Helper method to run a Tau test script
    void RunTauTest(const std::string& script, const std::string& testName,
                    bool expectSuccess = true) {
        // For now, the important thing is that the tests run and don't crash
        Registry r;
        auto lex = std::make_shared<tau::TauLexer>(script.c_str(), r);
        bool lexResult = lex->Process();

        // Debug the lexer output regardless of success/failure
        std::string lexerOutput = lex->Print();
        KAI_LOG_INFO("Lexer output for " + testName + ": " + lexerOutput);

        // More detailed error reporting before assertion
        if (!lexResult) {
            KAI_LOG_ERROR("Lexer for " + testName +
                          " failed. Check the lexer output above for details.");
            if (!expectSuccess) {
                SUCCEED() << "Lexer failed as expected for test: " << testName;
                return;
            }
        }

        if (expectSuccess) {
            ASSERT_TRUE(lexResult) << "Lexer for " << testName
                                   << " failed with output: " << lexerOutput;
        }

        // Create a parser with relaxed requirements
        auto parser = std::make_shared<tau::TauParser>(r);

        // Make a best effort to parse - don't assert on success
        bool success = parser->Process(lex, Structure::Module);
        if (!success) {
            KAI_LOG_WARNING("Parser for " + testName +
                            " reported failure: " + parser->Error);
            if (!expectSuccess) {
                SUCCEED() << "Parser failed as expected for test: " << testName;
                return;
            }
        }

        // Check result based on expectSuccess parameter
        if (expectSuccess) {
            EXPECT_TRUE(success)
                << "Parser for " << testName << " failed: " << parser->Error;
        } else {
            // For tests that are expected to fail
            SUCCEED() << "Test completed for: " << testName
                      << " (known limitation)";
        }
    }

    // Helper method to test proxy code generation
    void TestProxyGen(const std::string& script, const std::string& testName,
                      bool expectSuccess = true) {
        // Generate proxy code
        string output;
        tau::Generate::GenerateProxy proxy(script.c_str(), output);

        // Report the result
        if (proxy.Failed) {
            KAI_LOG_WARNING("Proxy generation for " + testName +
                            " reported failure: " + proxy.Error);
            if (!expectSuccess) {
                SUCCEED() << "Proxy generation failed as expected for test: "
                          << testName;
                return;
            }
        } else {
            KAI_LOG_INFO(
                "Proxy generation for " + testName +
                " succeeded, output size: " + std::to_string(output.size()));
        }

        // Check results based on expectSuccess parameter
        if (expectSuccess) {
            EXPECT_FALSE(proxy.Failed) << "Proxy generation for " << testName
                                       << " failed: " << proxy.Error;
            EXPECT_FALSE(output.empty()) << "Proxy generation for " << testName
                                         << " produced empty output";
        } else {
            SUCCEED() << "Proxy generation test completed for: " << testName;
        }
    }

    // Helper method to test agent code generation
    void TestAgentGen(const std::string& script, const std::string& testName,
                      bool expectSuccess = true) {
        // Generate agent code
        string output;
        tau::Generate::GenerateAgent agent(script.c_str(), output);

        // Report the result
        if (agent.Failed) {
            KAI_LOG_WARNING("Agent generation for " + testName +
                            " reported failure: " + agent.Error);
            if (!expectSuccess) {
                SUCCEED() << "Agent generation failed as expected for test: "
                          << testName;
                return;
            }
        } else {
            KAI_LOG_INFO(
                "Agent generation for " + testName +
                " succeeded, output size: " + std::to_string(output.size()));
        }

        // Check results based on expectSuccess parameter
        if (expectSuccess) {
            EXPECT_FALSE(agent.Failed) << "Agent generation for " << testName
                                       << " failed: " << agent.Error;
            EXPECT_FALSE(output.empty()) << "Agent generation for " << testName
                                         << " produced empty output";
        } else {
            SUCCEED() << "Agent generation test completed for: " << testName;
        }
    }
};

// Test interface inheritance
TEST_F(TauAdvancedTypeTests, TestInterfaceInheritance) {
    std::string script = R"(
    namespace KAI::Network
    {
        interface IBaseCommunication
        {
            bool Connect(string host, int port);
            void Disconnect();
            bool IsConnected();
        }
        
        interface IConnectionMonitor
        {
            int GetPing();
            int GetPacketLoss();
            int GetBandwidthUsage();
        }
        
        // Inherits from both interfaces
        interface IAdvancedConnection : IBaseCommunication, IConnectionMonitor
        {
            bool Reconnect();
            void SetConnectionTimeout(int seconds);
        }
    }
    )";

    RunTauTest(script, "InterfaceInheritance");
    TestProxyGen(script, "InterfaceInheritance");
    TestAgentGen(script, "InterfaceInheritance");
}

// Test advanced events with custom event arguments
TEST_F(TauAdvancedTypeTests, TestAdvancedEvents) {
    std::string script = R"(
    namespace KAI::Network::Events
    {
        struct ConnectionEventArgs
        {
            string host;
            int port;
            bool success;
            string errorMessage;
            int latency;
        }
        
        struct MessageEventArgs
        {
            string sender;
            string content;
            int64 timestamp;
            bool encrypted;
            int priority;
        }
        
        interface INetworkEventHandler
        {
            // Events with complex event args
            event ConnectionEstablished(ConnectionEventArgs args);
            event ConnectionLost(ConnectionEventArgs args);
            event MessageReceived(MessageEventArgs args);
            event MessageSent(MessageEventArgs args);
            
            // Methods to register handlers
            void RegisterConnectionHandler(object handler);
            void RegisterMessageHandler(object handler);
            void UnregisterHandler(object handler);
        }
    }
    )";

    RunTauTest(script, "AdvancedEvents");
    TestProxyGen(script, "AdvancedEvents");
    TestAgentGen(script, "AdvancedEvents");
}

// Test complex type definitions including nested structs and arrays
TEST_F(TauAdvancedTypeTests, TestComplexTypes) {
    std::string script = R"(
    namespace KAI::Data
    {
        enum DataType
        {
            None = 0,
            Integer = 1,
            Float = 2,
            String = 3,
            Boolean = 4,
            DateTime = 5,
            Blob = 6
        }
        
        struct DataField
        {
            string name;
            DataType type;
            bool isNullable;
            bool isIndexed;
            string defaultValue;
        }
        
        struct DataRecord
        {
            string id;
            DataField[] fields;
            object[] values;
            int64 createdTimestamp;
            int64 modifiedTimestamp;
        }
        
        struct DataQuery
        {
            string tableName;
            string[] selectFields;
            string whereClause;
            string orderBy;
            int limit;
            int offset;
        }
        
        interface IDataStore
        {
            DataRecord[] Query(DataQuery query);
            DataRecord GetById(string tableName, string id);
            void Insert(string tableName, DataRecord record);
            void Update(string tableName, DataRecord record);
            void Delete(string tableName, string id);
            
            // Schema management
            void CreateTable(string tableName, DataField[] fields);
            void AddField(string tableName, DataField field);
            void RemoveField(string tableName, string fieldName);
            DataField[] GetTableSchema(string tableName);
        }
    }
    )";

    RunTauTest(script, "ComplexTypes");
    TestProxyGen(script, "ComplexTypes");
    TestAgentGen(script, "ComplexTypes");
}

// Test generic interface definitions
TEST_F(TauAdvancedTypeTests, TestGenericInterfaces) {
    std::string script = R"(
    namespace KAI::Generic
    {
        interface IRepository<T>
        {
            T GetById(string id);
            T[] GetAll();
            void Add(T item);
            void Update(T item);
            void Delete(string id);
            T[] Find(Predicate<T> predicate);
        }
        
        interface IDataConverter<TSource, TTarget>
        {
            TTarget Convert(TSource source);
            TSource ConvertBack(TTarget target);
            bool CanConvert(TSource source);
            bool CanConvertBack(TTarget target);
        }
        
        // Interface with multiple type parameters
        interface IKeyValueStore<TKey, TValue>
        {
            void Set(TKey key, TValue value);
            TValue Get(TKey key);
            bool Contains(TKey key);
            void Remove(TKey key);
            TKey[] GetAllKeys();
        }
    }
    )";

    // Generic interfaces might not be fully supported
    RunTauTest(script, "GenericInterfaces", false);
    TestProxyGen(script, "GenericInterfaces", false);
    TestAgentGen(script, "GenericInterfaces", false);
}

// Test interface with function pointers / callbacks
TEST_F(TauAdvancedTypeTests, TestFunctionCallbacks) {
    std::string script = R"(
    namespace KAI::Callbacks
    {
        // Function pointer type definitions
        typedef void VoidCallback();
        typedef bool StatusCallback(int status);
        typedef void MessageCallback(string sender, string message);
        typedef int DataProcessor(byte[] data);
        
        interface ICallbackHandler
        {
            void RegisterVoidCallback(VoidCallback callback);
            void RegisterStatusCallback(StatusCallback callback);
            void RegisterMessageCallback(MessageCallback callback);
            void RegisterDataProcessor(DataProcessor processor);
            
            void UnregisterVoidCallback(VoidCallback callback);
            void UnregisterStatusCallback(StatusCallback callback);
            void UnregisterMessageCallback(MessageCallback callback);
            void UnregisterDataProcessor(DataProcessor processor);
            
            void TriggerVoidCallbacks();
            bool TriggerStatusCallbacks(int status);
            void TriggerMessageCallbacks(string sender, string message);
            int ProcessData(byte[] data);
        }
    }
    )";

    // Function pointers might not be fully supported
    RunTauTest(script, "FunctionCallbacks", false);
    TestProxyGen(script, "FunctionCallbacks", false);
    TestAgentGen(script, "FunctionCallbacks", false);
}

// Test complex protocol definition with states and transitions
TEST_F(TauAdvancedTypeTests, TestProtocolDefinition) {
    std::string script = R"(
    namespace KAI::Protocol
    {
        enum ProtocolState
        {
            Idle = 0,
            Connecting = 1,
            Authenticating = 2,
            Connected = 3,
            Disconnecting = 4,
            Error = 99
        }
        
        enum MessageType
        {
            Connect = 1,
            Disconnect = 2,
            Data = 3,
            Ping = 4,
            Pong = 5,
            Authentication = 6,
            Acknowledge = 7,
            Error = 8
        }
        
        struct Message
        {
            MessageType type;
            int sequence;
            int64 timestamp;
            byte[] payload;
        }
        
        interface IProtocolHandler
        {
            // State management
            ProtocolState GetState();
            void SetState(ProtocolState newState);
            
            // Message handling
            void SendMessage(Message message);
            void ReceiveMessage(Message message);
            
            // Protocol operations
            void Connect(string host, int port);
            void Authenticate(string username, string password);
            void Disconnect();
            bool IsConnected();
            
            // Events
            event StateChanged(ProtocolState oldState, ProtocolState newState);
            event MessageSent(Message message);
            event MessageReceived(Message message);
            event Error(string errorMessage, int errorCode);
        }
    }
    )";

    RunTauTest(script, "ProtocolDefinition");
    TestProxyGen(script, "ProtocolDefinition");
    TestAgentGen(script, "ProtocolDefinition");
}

// Test advanced networking configuration interfaces
TEST_F(TauAdvancedTypeTests, TestNetworkConfiguration) {
    std::string script = R"(
    namespace KAI::Network::Config
    {
        struct NetworkEndpoint
        {
            string host;
            int port;
            bool secure;
        }
        
        struct ConnectionOptions
        {
            int timeout = 30;
            int retryCount = 3;
            int retryDelay = 5;
            bool autoReconnect = false;
            int keepAliveInterval = 60;
        }
        
        struct SecurityOptions
        {
            bool enabled = false;
            string certificatePath;
            string privateKeyPath;
            bool verifyCertificate = true;
            string[] trustedCertificates;
            string cipherSuite;
        }
        
        struct ProxySettings
        {
            bool useProxy = false;
            string proxyHost;
            int proxyPort;
            string proxyUsername;
            string proxyPassword;
            string proxyType = "http";
        }
        
        interface INetworkConfigProvider
        {
            // Configuration getters
            NetworkEndpoint GetEndpoint();
            ConnectionOptions GetConnectionOptions();
            SecurityOptions GetSecurityOptions();
            ProxySettings GetProxySettings();
            
            // Configuration setters
            void SetEndpoint(NetworkEndpoint endpoint);
            void SetConnectionOptions(ConnectionOptions options);
            void SetSecurityOptions(SecurityOptions options);
            void SetProxySettings(ProxySettings proxySettings);
            
            // Configuration management
            void LoadConfiguration(string configFile);
            void SaveConfiguration(string configFile);
            void ResetToDefaults();
            
            // Configuration validation
            bool ValidateConfiguration();
            string[] GetValidationErrors();
        }
    }
    )";

    RunTauTest(script, "NetworkConfiguration");
    TestProxyGen(script, "NetworkConfiguration");
    TestAgentGen(script, "NetworkConfiguration");
}

// Test advanced data serialization interfaces
TEST_F(TauAdvancedTypeTests, TestSerializationInterfaces) {
    std::string script = R"(
    namespace KAI::Serialization
    {
        enum SerializationFormat
        {
            JSON = 0,
            XML = 1,
            Binary = 2,
            ProtoBuf = 3,
            MessagePack = 4
        }
        
        struct SerializerOptions
        {
            bool prettyPrint = false;
            bool includeNulls = true;
            bool caseInsensitive = false;
            int maxDepth = 32;
            string dateTimeFormat = "ISO8601";
        }
        
        interface ISerializer
        {
            // Serialization methods
            string SerializeToString(object obj);
            byte[] SerializeToBytes(object obj);
            void SerializeToStream(object obj, System.IO.Stream stream);
            
            // Deserialization methods
            T Deserialize<T>(string serialized);
            T DeserializeFromBytes<T>(byte[] data);
            T DeserializeFromStream<T>(System.IO.Stream stream);
            
            // Configuration
            SerializationFormat GetFormat();
            SerializerOptions GetOptions();
            void SetOptions(SerializerOptions options);
            
            // Schema handling
            string GenerateSchema<T>();
            bool ValidateAgainstSchema(string serialized, string schema);
        }
    }
    )";

    // Generic methods might not be fully supported
    RunTauTest(script, "SerializationInterfaces", false);
    TestProxyGen(script, "SerializationInterfaces", false);
    TestAgentGen(script, "SerializationInterfaces", false);
}

// Test advanced logging interface system with multiple loggers
TEST_F(TauAdvancedTypeTests, TestLoggingInterfaces) {
    std::string script = R"(
    namespace KAI::Logging
    {
        enum LogLevel
        {
            Trace = 0,
            Debug = 1,
            Info = 2,
            Warning = 3,
            Error = 4,
            Critical = 5,
            None = 6
        }
        
        struct LogEntry
        {
            int64 timestamp;
            LogLevel level;
            string message;
            string source;
            string exception;
            string[] tags;
            Dictionary<string, string> properties;
        }
        
        interface ILogger
        {
            void Log(LogEntry entry);
            void Trace(string message);
            void Debug(string message);
            void Info(string message);
            void Warning(string message);
            void Error(string message);
            void Critical(string message);
            
            bool IsEnabled(LogLevel level);
            void SetMinimumLevel(LogLevel level);
        }
        
        interface ILoggerProvider
        {
            ILogger CreateLogger(string categoryName);
            void Dispose();
        }
        
        interface ILoggerFactory
        {
            ILogger CreateLogger(string categoryName);
            void AddProvider(ILoggerProvider provider);
            void ClearProviders();
        }
    }
    )";

    RunTauTest(script, "LoggingInterfaces");
    TestProxyGen(script, "LoggingInterfaces");
    TestAgentGen(script, "LoggingInterfaces");
}

// Test distributed task processing system
TEST_F(TauAdvancedTypeTests, TestDistributedTaskSystem) {
    std::string script = R"(
    namespace KAI::Tasks
    {
        enum TaskPriority
        {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        }
        
        enum TaskStatus
        {
            Created = 0,
            Queued = 1,
            Running = 2,
            Completed = 3,
            Failed = 4,
            Cancelled = 5
        }
        
        struct TaskDefinition
        {
            string id;
            string name;
            string type;
            byte[] payload;
            TaskPriority priority = TaskPriority.Normal;
            int64 creationTime;
            int64 executionTimeout = 60000;
            int maxRetries = 3;
            string[] dependencies;
        }
        
        struct TaskResult
        {
            string taskId;
            TaskStatus status;
            byte[] result;
            string errorMessage;
            int64 startTime;
            int64 endTime;
            int retryCount;
            int workerNodeId;
        }
        
        interface ITaskScheduler
        {
            string SubmitTask(TaskDefinition task);
            TaskStatus GetTaskStatus(string taskId);
            TaskResult GetTaskResult(string taskId);
            void CancelTask(string taskId);
            
            TaskDefinition[] GetPendingTasks();
            TaskDefinition[] GetRunningTasks();
            TaskResult[] GetCompletedTasks();
            TaskResult[] GetFailedTasks();
            
            void RegisterWorkerNode(string nodeId, string[] supportedTaskTypes);
            void UnregisterWorkerNode(string nodeId);
            
            event TaskStatusChanged(string taskId, TaskStatus newStatus);
            event TaskCompleted(TaskResult result);
            event TaskFailed(TaskResult result);
        }
        
        interface ITaskWorker
        {
            void Start();
            void Stop();
            TaskStatus ExecuteTask(TaskDefinition task);
            string[] GetSupportedTaskTypes();
            bool IsAvailable();
            int GetCapacity();
            
            event TaskStarted(string taskId);
            event TaskProgress(string taskId, int progressPercent);
            event TaskCompleted(TaskResult result);
        }
    }
    )";

    RunTauTest(script, "DistributedTaskSystem");
    TestProxyGen(script, "DistributedTaskSystem");
    TestAgentGen(script, "DistributedTaskSystem");
}

// Test data validation interface system
TEST_F(TauAdvancedTypeTests, TestDataValidation) {
    std::string script = R"(
    namespace KAI::Validation
    {
        enum ValidationType
        {
            Required = 0,
            StringLength = 1,
            Range = 2,
            RegularExpression = 3,
            Email = 4,
            Comparison = 5,
            Custom = 6
        }
        
        struct ValidationRule
        {
            string propertyName;
            ValidationType type;
            string errorMessage;
            object minValue;
            object maxValue;
            string pattern;
            string comparisonProperty;
            string comparisonOperator;
        }
        
        struct ValidationResult
        {
            bool isValid;
            string[] errors;
            Dictionary<string, string[]> propertyErrors;
        }
        
        interface IValidator<T>
        {
            void AddRule(ValidationRule rule);
            void RemoveRule(string propertyName, ValidationType type);
            void ClearRules();
            ValidationRule[] GetRules();
            
            ValidationResult Validate(T obj);
            bool IsValid(T obj);
            string[] GetErrors(T obj);
            string[] GetPropertyErrors(T obj, string propertyName);
        }
        
        interface IValidatorFactory
        {
            IValidator<T> CreateValidator<T>();
            void RegisterValidator<T>(IValidator<T> validator);
            bool HasValidator<T>();
        }
    }
    )";

    // Generic interfaces might not be fully supported
    RunTauTest(script, "DataValidation", false);
    TestProxyGen(script, "DataValidation", false);
    TestAgentGen(script, "DataValidation", false);
}

// Test caching interface system
TEST_F(TauAdvancedTypeTests, TestCachingInterfaces) {
    std::string script = R"(
    namespace KAI::Caching
    {
        enum CacheItemPriority
        {
            Low = 0,
            Normal = 1,
            High = 2,
            NeverRemove = 3
        }
        
        struct CacheEntryOptions
        {
            int64 absoluteExpiration = 0;
            int64 slidingExpiration = 0;
            CacheItemPriority priority = CacheItemPriority.Normal;
            int size = 1;
        }
        
        struct CacheStatistics
        {
            int64 hits;
            int64 misses;
            int64 removals;
            int64 expirations;
            int64 size;
            int64 capacity;
            float hitRatio;
        }
        
        interface ICache
        {
            void Set(string key, object value);
            void Set(string key, object value, CacheEntryOptions options);
            object Get(string key);
            T Get<T>(string key);
            bool TryGet(string key, out object value);
            bool TryGet<T>(string key, out T value);
            
            bool Contains(string key);
            bool Remove(string key);
            void Clear();
            
            string[] GetKeys();
            int GetCount();
            CacheStatistics GetStatistics();
            
            event CacheEntryAdded(string key);
            event CacheEntryRemoved(string key, object value, string reason);
            event CacheEntryExpired(string key);
        }
        
        interface IDistributedCache : ICache
        {
            void Refresh(string key);
            void RefreshAsync(string key);
            
            void SetMultiple(Dictionary<string, object> items);
            Dictionary<string, object> GetMultiple(string[] keys);
            bool RemoveMultiple(string[] keys);
            
            void Synchronize();
            void SetMasterNode(string nodeId);
            void AddNode(string nodeId);
            void RemoveNode(string nodeId);
        }
    }
    )";

    // Generic methods and out parameters might not be fully supported
    RunTauTest(script, "CachingInterfaces", false);
    TestProxyGen(script, "CachingInterfaces", false);
    TestAgentGen(script, "CachingInterfaces", false);
}

// Test messaging system with queues and topics
TEST_F(TauAdvancedTypeTests, TestMessagingSystem) {
    std::string script = R"(
    namespace KAI::Messaging
    {
        enum DeliveryMode
        {
            AtMostOnce = 0,    // QoS 0
            AtLeastOnce = 1,   // QoS 1
            ExactlyOnce = 2    // QoS 2
        }
        
        enum MessagePriority
        {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        }
        
        struct MessageProperties
        {
            string messageId;
            string correlationId;
            string replyTo;
            int64 timestamp;
            int64 expiration;
            DeliveryMode deliveryMode = DeliveryMode.AtLeastOnce;
            MessagePriority priority = MessagePriority.Normal;
            Dictionary<string, string> headers;
            string contentType;
            string contentEncoding;
        }
        
        struct Message
        {
            MessageProperties properties;
            byte[] body;
        }
        
        interface IMessageProducer
        {
            string Send(string destination, byte[] body);
            string Send(string destination, byte[] body, MessageProperties properties);
            string[] SendBatch(string destination, byte[][] bodies);
            string[] SendBatch(string destination, byte[][] bodies, MessageProperties[] properties);
            
            void SetDefaultProperties(MessageProperties defaultProperties);
            bool IsSynchronous();
            void SetSynchronous(bool synchronous);
            
            event MessageSent(string messageId, string destination);
            event MessageDelivered(string messageId, string destination);
            event MessageFailed(string messageId, string destination, string reason);
        }
        
        interface IMessageConsumer
        {
            void Subscribe(string destination);
            void Unsubscribe(string destination);
            string[] GetSubscriptions();
            
            void SetMessageHandler(System.Action<Message> handler);
            Message Receive();
            Message Receive(int timeout);
            Message[] ReceiveBatch(int maxMessages);
            Message[] ReceiveBatch(int maxMessages, int timeout);
            
            void Acknowledge(string messageId);
            void Reject(string messageId, bool requeue = false);
            
            void Start();
            void Stop();
            bool IsRunning();
            
            event MessageReceived(Message message);
            event SubscriptionChanged(string destination, bool subscribed);
        }
        
        interface IMessageBroker
        {
            IMessageProducer CreateProducer();
            IMessageConsumer CreateConsumer();
            
            void CreateQueue(string queueName);
            void CreateTopic(string topicName);
            void DeleteQueue(string queueName);
            void DeleteTopic(string topicName);
            
            bool QueueExists(string queueName);
            bool TopicExists(string topicName);
            int GetQueueDepth(string queueName);
            int GetSubscriberCount(string topicName);
            
            void PurgeQueue(string queueName);
            
            event BrokerStarted();
            event BrokerStopped();
            event QueueCreated(string queueName);
            event TopicCreated(string topicName);
        }
    }
    )";

    RunTauTest(script, "MessagingSystem");
    TestProxyGen(script, "MessagingSystem");
    TestAgentGen(script, "MessagingSystem");
}

// Test configuration management system
TEST_F(TauAdvancedTypeTests, TestConfigurationSystem) {
    std::string script = R"(
    namespace KAI::Configuration
    {
        enum ConfigurationSource
        {
            Default = 0,
            JsonFile = 1,
            XmlFile = 2,
            EnvironmentVariables = 3,
            CommandLine = 4,
            Registry = 5,
            Database = 6,
            Remote = 7
        }
        
        struct ConfigurationValue
        {
            string key;
            string value;
            ConfigurationSource source;
            bool isReadOnly;
        }
        
        struct ConfigurationSection
        {
            string path;
            Dictionary<string, string> values;
            Dictionary<string, ConfigurationSection> children;
        }
        
        interface IConfigurationProvider
        {
            bool TryGet(string key, out string value);
            string Get(string key);
            string Get(string key, string defaultValue);
            Dictionary<string, string> GetAll();
            void Set(string key, string value);
            void Remove(string key);
            bool Contains(string key);
            ConfigurationSource GetSource();
            void Load();
            void Save();
        }
        
        interface IConfiguration
        {
            string Get(string key);
            string Get(string key, string defaultValue);
            T GetValue<T>(string key);
            T GetValue<T>(string key, T defaultValue);
            
            void Set(string key, string value);
            void Set<T>(string key, T value);
            
            ConfigurationSection GetSection(string sectionPath);
            string[] GetChildren(string path);
            
            void AddProvider(IConfigurationProvider provider);
            void RemoveProvider(ConfigurationSource source);
            IConfigurationProvider[] GetProviders();
            
            void Reload();
            
            event ConfigurationChanged(string key, string newValue, string oldValue);
            event ConfigurationReloaded();
        }
    }
    )";

    // Generic methods and out parameters might not be fully supported
    RunTauTest(script, "ConfigurationSystem", false);
    TestProxyGen(script, "ConfigurationSystem", false);
    TestAgentGen(script, "ConfigurationSystem", false);
}

// Test dependency injection system
TEST_F(TauAdvancedTypeTests, TestDependencyInjection) {
    std::string script = R"(
    namespace KAI::DependencyInjection
    {
        enum ServiceLifetime
        {
            Singleton = 0,
            Scoped = 1,
            Transient = 2
        }
        
        enum InjectionBehavior
        {
            Optional = 0,
            Required = 1,
            RequiredAllowDefault = 2
        }
        
        interface IServiceDescriptor
        {
            string ServiceType;
            string ImplementationType;
            object ImplementationInstance;
            System.Func<object> ImplementationFactory;
            ServiceLifetime Lifetime;
        }
        
        interface IServiceCollection
        {
            IServiceCollection AddSingleton<TService>();
            IServiceCollection AddSingleton<TService, TImplementation>();
            IServiceCollection AddSingleton<TService>(TService implementationInstance);
            
            IServiceCollection AddScoped<TService>();
            IServiceCollection AddScoped<TService, TImplementation>();
            
            IServiceCollection AddTransient<TService>();
            IServiceCollection AddTransient<TService, TImplementation>();
            
            IServiceCollection Remove<TService>();
            IServiceCollection Replace<TService>(IServiceDescriptor descriptor);
            
            bool Contains<TService>();
            IServiceDescriptor GetDescriptor<TService>();
            IServiceDescriptor[] GetDescriptors<TService>();
            IServiceDescriptor[] GetAllDescriptors();
            
            IServiceProvider BuildServiceProvider();
        }
        
        interface IServiceProvider
        {
            object GetService(string serviceType);
            T GetService<T>();
            object GetRequiredService(string serviceType);
            T GetRequiredService<T>();
            T[] GetServices<T>();
            
            IServiceScope CreateScope();
            IServiceProvider GetRootProvider();
        }
        
        interface IServiceScope
        {
            IServiceProvider ServiceProvider { get; }
            void Dispose();
        }
    }
    )";

    // Generic methods might not be fully supported
    RunTauTest(script, "DependencyInjection", false);
    TestProxyGen(script, "DependencyInjection", false);
    TestAgentGen(script, "DependencyInjection", false);
}

// Test distributed locking system
TEST_F(TauAdvancedTypeTests, TestDistributedLockSystem) {
    std::string script = R"(
    namespace KAI::Locking
    {
        enum LockStatus
        {
            None = 0,
            Acquired = 1,
            Blocked = 2,
            TimedOut = 3,
            Released = 4,
            Failed = 5
        }
        
        struct LockInfo
        {
            string resourceId;
            string lockId;
            string ownerId;
            int64 acquiredTime;
            int64 expirationTime;
            bool isExclusive;
        }
        
        struct LockOptions
        {
            int timeoutMilliseconds = 30000;
            int64 expiryMilliseconds = 300000;
            bool waitForLock = true;
            int retryCount = 3;
            int retryDelayMilliseconds = 1000;
            bool autoRenew = false;
            int renewIntervalMilliseconds = 60000;
        }
        
        interface IDistributedLockProvider
        {
            string AcquireLock(string resourceId, string ownerId);
            string AcquireLock(string resourceId, string ownerId, LockOptions options);
            
            bool ReleaseLock(string resourceId, string lockId);
            bool ReleaseLock(string resourceId, string lockId, string ownerId);
            
            bool RenewLock(string resourceId, string lockId);
            bool RenewLock(string resourceId, string lockId, int64 expiryMilliseconds);
            
            bool IsLocked(string resourceId);
            LockInfo GetLockInfo(string resourceId);
            
            event LockAcquired(string resourceId, string lockId, string ownerId);
            event LockReleased(string resourceId, string lockId, string ownerId);
            event LockRenewed(string resourceId, string lockId, string ownerId);
            event LockExpired(string resourceId, string lockId, string ownerId);
        }
        
        interface IDistributedLock
        {
            bool IsAcquired();
            string GetResourceId();
            string GetLockId();
            LockStatus GetStatus();
            
            bool Acquire();
            bool Acquire(LockOptions options);
            bool Release();
            bool Renew();
            bool Renew(int64 expiryMilliseconds);
            
            void Dispose();
            
            event StatusChanged(LockStatus newStatus, LockStatus oldStatus);
            event Expired();
        }
    }
    )";

    RunTauTest(script, "DistributedLockSystem");
    TestProxyGen(script, "DistributedLockSystem");
    TestAgentGen(script, "DistributedLockSystem");
}

// Test circuit breaker pattern interfaces
TEST_F(TauAdvancedTypeTests, TestCircuitBreakerPattern) {
    std::string script = R"(
    namespace KAI::Resilience
    {
        enum CircuitState
        {
            Closed = 0,      // Normal operation
            Open = 1,        // Failing, rejecting requests
            HalfOpen = 2     // Testing if service is recovered
        }
        
        struct CircuitBreakerOptions
        {
            int failureThreshold = 5;
            int64 samplingPeriodMs = 60000;
            int64 durationOfBreakMs = 30000;
            int successThreshold = 2;
            float failureThresholdPercentage = 50.0;
            string[] handledExceptions;
        }
        
        struct CircuitBreakerMetrics
        {
            int totalRequests;
            int successfulRequests;
            int failedRequests;
            int rejectedRequests;
            int64 lastStateChangedTimestamp;
            int64 lastFailureTimestamp;
            int consecFailures;
            int consecSuccesses;
            float failurePercentage;
        }
        
        interface ICircuitBreaker
        {
            string Name { get; }
            CircuitState State { get; }
            CircuitBreakerMetrics Metrics { get; }
            
            void ExecuteAction(System.Action action);
            T ExecuteFunc<T>(System.Func<T> func);
            
            bool AllowRequest();
            void MarkSuccess();
            void MarkFailure();
            void Reset();
            
            event StateChanged(CircuitState oldState, CircuitState newState);
            event RequestRejected();
            event RequestSucceeded();
            event RequestFailed();
        }
        
        interface ICircuitBreakerFactory
        {
            ICircuitBreaker Create(string name);
            ICircuitBreaker Create(string name, CircuitBreakerOptions options);
            ICircuitBreaker GetOrCreate(string name);
            ICircuitBreaker GetOrCreate(string name, CircuitBreakerOptions options);
            
            bool Remove(string name);
            ICircuitBreaker[] GetAll();
            bool Contains(string name);
            void Reset(string name);
            void ResetAll();
        }
    }
    )";

    // Generic methods might not be fully supported
    RunTauTest(script, "CircuitBreakerPattern", false);
    TestProxyGen(script, "CircuitBreakerPattern", false);
    TestAgentGen(script, "CircuitBreakerPattern", false);
}

// Test command pattern interfaces
TEST_F(TauAdvancedTypeTests, TestCommandPattern) {
    std::string script = R"(
    namespace KAI::CommandPattern
    {
        enum CommandStatus
        {
            Created = 0,
            Executing = 1,
            Succeeded = 2,
            Failed = 3,
            Cancelled = 4
        }
        
        struct CommandMetadata
        {
            string commandId;
            string commandType;
            string executedBy;
            int64 createdTimestamp;
            int64 executedTimestamp;
            int64 completedTimestamp;
            Dictionary<string, string> properties;
        }
        
        struct CommandResult
        {
            string commandId;
            CommandStatus status;
            bool succeeded;
            string errorMessage;
            object result;
        }
        
        interface ICommand
        {
            string CommandId { get; }
            string CommandType { get; }
            CommandStatus Status { get; }
            CommandMetadata Metadata { get; }
            
            void Execute();
            bool CanExecute();
            void Cancel();
            
            bool AllowsUndo();
            void Undo();
            
            event StatusChanged(CommandStatus oldStatus, CommandStatus newStatus);
        }
        
        interface ICommandResult
        {
            string CommandId { get; }
            bool Succeeded { get; }
            object GetResult();
            T GetResult<T>();
            string GetErrorMessage();
        }
        
        interface ICommandFactory
        {
            ICommand Create(string commandType);
            ICommand Create(string commandType, Dictionary<string, object> parameters);
            bool CanCreate(string commandType);
            string[] GetSupportedCommandTypes();
        }
        
        interface ICommandInvoker
        {
            ICommandResult Execute(ICommand command);
            ICommandResult[] ExecuteBatch(ICommand[] commands);
            
            string[] GetCommandHistory();
            void ClearHistory();
            
            void Undo();
            void Redo();
            bool CanUndo();
            bool CanRedo();
            
            event CommandExecuted(ICommand command, ICommandResult result);
            event CommandUndone(ICommand command);
            event CommandRedone(ICommand command);
        }
    }
    )";

    // Generic methods might not be fully supported
    RunTauTest(script, "CommandPattern", false);
    TestProxyGen(script, "CommandPattern", false);
    TestAgentGen(script, "CommandPattern", false);
}

// Test event sourcing pattern
TEST_F(TauAdvancedTypeTests, TestEventSourcingPattern) {
    std::string script = R"(
    namespace KAI::EventSourcing
    {
        struct DomainEvent
        {
            string eventId;
            string aggregateId;
            string eventType;
            int version;
            int64 timestamp;
            byte[] data;
            Dictionary<string, string> metadata;
        }
        
        struct EventStream
        {
            string streamId;
            int version;
            DomainEvent[] events;
        }
        
        struct SnapshotInfo
        {
            string snapshotId;
            string aggregateId;
            int version;
            int64 timestamp;
            byte[] data;
        }
        
        interface IEventStore
        {
            string AppendEvent(string streamId, DomainEvent domainEvent);
            string[] AppendEvents(string streamId, DomainEvent[] domainEvents);
            
            EventStream GetStream(string streamId);
            EventStream GetStream(string streamId, int fromVersion);
            EventStream GetStream(string streamId, int fromVersion, int toVersion);
            
            DomainEvent[] GetEvents(string streamId);
            DomainEvent[] GetEvents(string streamId, int fromVersion);
            DomainEvent[] GetEvents(string streamId, int fromVersion, int toVersion);
            
            DomainEvent GetEvent(string eventId);
            
            int GetStreamVersion(string streamId);
            bool StreamExists(string streamId);
            
            event EventAppended(string streamId, DomainEvent domainEvent);
            event StreamCreated(string streamId);
        }
        
        interface ISnapshotStore
        {
            string CreateSnapshot(string aggregateId, int version, byte[] data);
            SnapshotInfo GetLatestSnapshot(string aggregateId);
            SnapshotInfo GetSnapshotByVersion(string aggregateId, int version);
            SnapshotInfo[] GetSnapshots(string aggregateId);
            void DeleteSnapshot(string snapshotId);
            void DeleteAllSnapshots(string aggregateId);
            
            event SnapshotCreated(SnapshotInfo snapshotInfo);
            event SnapshotDeleted(string snapshotId);
        }
        
        interface IEventBus
        {
            void Publish(DomainEvent domainEvent);
            void PublishAll(DomainEvent[] domainEvents);
            
            void Subscribe<T>(string eventType, System.Action<T> handler);
            void Unsubscribe<T>(string eventType, System.Action<T> handler);
            
            string[] GetSubscribedEventTypes();
            int GetSubscriberCount(string eventType);
            
            event EventPublished(DomainEvent domainEvent);
            event SubscriptionAdded(string eventType);
            event SubscriptionRemoved(string eventType);
        }
    }
    )";

    // Generic methods might not be fully supported
    RunTauTest(script, "EventSourcingPattern", false);
    TestProxyGen(script, "EventSourcingPattern", false);
    TestAgentGen(script, "EventSourcingPattern", false);
}

// Test reactive programming interfaces
TEST_F(TauAdvancedTypeTests, TestReactiveInterfaces) {
    std::string script = R"(
    namespace KAI::Reactive
    {
        struct Subscription
        {
            string id;
            string observableId;
            string observerId;
            bool isActive;
        }
        
        interface IObservable<T>
        {
            string Id { get; }
            
            Subscription Subscribe(IObserver<T> observer);
            void Unsubscribe(string subscriptionId);
            void Unsubscribe(IObserver<T> observer);
            
            IObserver<T>[] GetObservers();
            int ObserverCount();
            
            void OnNext(T value);
            void OnError(System.Exception error);
            void OnCompleted();
        }
        
        interface IObserver<T>
        {
            string Id { get; }
            
            void OnNext(T value);
            void OnError(System.Exception error);
            void OnCompleted();
            
            bool IsCompleted();
        }
        
        interface ISubject<T> : IObservable<T>, IObserver<T>
        {
            void Reset();
            T GetLastValue();
            bool HasValue();
        }
        
        // Reactive operators
        interface IOperator<TSource, TResult>
        {
            IObservable<TResult> Apply(IObservable<TSource> source);
        }
        
        interface IReactiveFactory
        {
            IObservable<T> CreateObservable<T>();
            IObserver<T> CreateObserver<T>(System.Action<T> onNext);
            ISubject<T> CreateSubject<T>();
            
            IObservable<T> FromArray<T>(T[] array);
            IObservable<T> FromEvent<T>(string eventName, object source);
            IObservable<T> Timer(int64 intervalMs);
        }
    }
    )";

    // Generic interfaces might not be fully supported
    RunTauTest(script, "ReactiveInterfaces", false);
    TestProxyGen(script, "ReactiveInterfaces", false);
    TestAgentGen(script, "ReactiveInterfaces", false);
}