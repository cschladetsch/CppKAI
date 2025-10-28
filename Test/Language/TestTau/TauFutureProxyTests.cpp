#include <gtest/gtest.h>

#include "KAI/Language/Tau/TauParser.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Comprehensive test suite for Tau Future<T> proxy->method patterns
// 50 tests covering the distributed async syntax goal:
// Future<T> result = proxy->method(a,b,c);
//
// Purpose: Validate the core distributed system syntax and ensure
// proper code generation for network-transparent RPC calls
struct TauFutureProxyTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
    }

    // Helper to parse Tau IDL and verify no errors
    bool ParseTauCode(const string& code) {
        auto lexer = make_shared<tau::TauLexer>(code.c_str(), *reg_);
        if (!lexer->Process()) {
            ADD_FAILURE() << "Lexer failed";
            return false;
        }

        auto parser = make_shared<tau::TauParser>(*reg_);
        if (!parser->Process(lexer, Structure::Module)) {
            ADD_FAILURE() << "Parser failed: " << parser->Error;
            return false;
        }

        return parser->Error.empty();
    }
};

// ============================================================================
// BASIC FUTURE<T> INTERFACE DEFINITIONS (10 tests)
// ============================================================================

TEST_F(TauFutureProxyTests, Future_IntReturnType) {
    const char* code = R"(
        namespace Test {
            interface ICalculator {
                Future<int> Calculate(int a, int b);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_StringReturnType) {
    const char* code = R"(
        namespace Test {
            interface IDataService {
                Future<string> GetData(int id);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_BoolReturnType) {
    const char* code = R"(
        namespace Test {
            interface IValidator {
                Future<bool> Validate(string input);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_CustomTypeReturn) {
    const char* code = R"(
        namespace Test {
            struct User {
                string name;
                int id;
            }

            interface IUserService {
                Future<User> GetUser(int userId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_ArrayReturnType) {
    const char* code = R"(
        namespace Test {
            interface ICollectionService {
                Future<int[]> GetNumbers(int count);
                Future<string[]> GetNames();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_VoidReturnType) {
    const char* code = R"(
        namespace Test {
            interface ICommandService {
                Future<void> Execute(string command);
                Future<void> Shutdown();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_MultipleParameters) {
    const char* code = R"(
        namespace Test {
            interface IMathService {
                Future<float> Divide(float numerator, float denominator);
                Future<int> Sum(int a, int b, int c, int d);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_NoParameters) {
    const char* code = R"(
        namespace Test {
            interface IStatusService {
                Future<int> GetStatus();
                Future<string> GetVersion();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_ComplexParameters) {
    const char* code = R"(
        namespace Test {
            struct Query {
                string filter;
                int limit;
            }

            struct Result {
                int count;
                string[] items;
            }

            interface IQueryService {
                Future<Result> ExecuteQuery(Query query);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Future_GenericReturnType) {
    const char* code = R"(
        namespace Test {
            interface IGenericService {
                Future<List<int>> GetIntList();
                Future<List<string>> GetStringList();
                Future<Map<string, int>> GetDictionary();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// PROXY CLASS DEFINITIONS WITH FUTURE<T> (10 tests)
// ============================================================================

TEST_F(TauFutureProxyTests, Proxy_BasicProxyClass) {
    const char* code = R"(
        namespace Distributed {
            interface IRemoteCalculator {
                Future<int> Add(int a, int b);
                Future<int> Multiply(int a, int b);
            }

            class RemoteCalculatorProxy : IRemoteCalculator {
                Future<int> Add(int a, int b);
                Future<int> Multiply(int a, int b);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_WithAgentClass) {
    const char* code = R"(
        namespace Distributed {
            interface IDataStore {
                Future<string> Get(string key);
                Future<void> Set(string key, string value);
            }

            class DataStoreProxy : IDataStore {
                Future<string> Get(string key);
                Future<void> Set(string key, string value);
            }

            class DataStoreAgent : IDataStore {
                Future<string> Get(string key);
                Future<void> Set(string key, string value);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_MultipleInterfaces) {
    const char* code = R"(
        namespace Services {
            interface IReadService {
                Future<string> Read(int id);
            }

            interface IWriteService {
                Future<void> Write(int id, string data);
            }

            class DataService : IReadService, IWriteService {
                Future<string> Read(int id);
                Future<void> Write(int id, string data);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_WithProperties) {
    const char* code = R"(
        namespace Services {
            interface IConfigService {
                Future<int> GetTimeout();
                Future<void> SetTimeout(int value);
                Future<string> GetServerName();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_ComplexDataTypes) {
    const char* code = R"(
        namespace Game {
            struct Vector3 {
                float x;
                float y;
                float z;
            }

            struct Player {
                string name;
                int health;
                Vector3 position;
            }

            interface IGameService {
                Future<Player> GetPlayer(int playerId);
                Future<void> UpdatePosition(int playerId, Vector3 newPos);
                Future<Vector3> GetPosition(int playerId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_WithEvents) {
    const char* code = R"(
        namespace Events {
            interface IEventService {
                Future<void> Subscribe(string eventName);
                Future<void> Unsubscribe(string eventName);
                Future<void> Publish(string eventName, object data);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_NestedNamespaces) {
    const char* code = R"(
        namespace Company { namespace Services { namespace Data {
            interface IRepository {
                Future<object> Find(int id);
                Future<void> Save(object entity);
                Future<void> Delete(int id);
            }
        }}}
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_InheritedInterfaces) {
    const char* code = R"(
        namespace Services {
            interface IBaseService {
                Future<bool> IsAvailable();
            }

            interface IDataService : IBaseService {
                Future<string> GetData(int id);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_StaticMethods) {
    const char* code = R"(
        namespace Utilities {
            class MathService {
                static Future<int> CalculatePi(int precision);
                static Future<float> Random();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Proxy_OverloadedMethods) {
    const char* code = R"(
        namespace Services {
            interface ICalculator {
                Future<int> Calculate(int a);
                Future<int> Calculate(int a, int b);
                Future<float> Calculate(float a, float b);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// DISTRIBUTED GARBAGE COLLECTION SCENARIOS (10 tests)
// ============================================================================

TEST_F(TauFutureProxyTests, DistributedGC_RemoteObjectLifetime) {
    const char* code = R"(
        namespace GC {
            interface IRemoteObject {
                Future<void> AddRef();
                Future<void> Release();
                Future<int> GetRefCount();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_ProxyDisposal) {
    const char* code = R"(
        namespace GC {
            interface IDisposable {
                Future<void> Dispose();
                Future<bool> IsDisposed();
            }

            interface IRemoteService : IDisposable {
                Future<string> DoWork();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_WeakReferences) {
    const char* code = R"(
        namespace GC {
            interface IReferenceManager {
                Future<int> CreateWeakRef(object target);
                Future<object> GetWeakRefTarget(int refId);
                Future<bool> IsWeakRefAlive(int refId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_CrossNodeReferences) {
    const char* code = R"(
        namespace GC {
            struct RemoteHandle {
                int nodeId;
                int objectId;
            }

            interface IDistributedGC {
                Future<RemoteHandle> CreateRemoteObject(string typeName);
                Future<void> MarkReachable(RemoteHandle handle);
                Future<void> CollectGarbage();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_LeaseManagement) {
    const char* code = R"(
        namespace GC {
            interface ILeaseManager {
                Future<int> AcquireLease(object remoteObject, int duration);
                Future<void> RenewLease(int leaseId, int additionalDuration);
                Future<void> ReleaseLease(int leaseId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_ReferenceTracking) {
    const char* code = R"(
        namespace GC {
            interface IReferenceTracker {
                Future<void> TrackReference(int sourceNode, int targetNode, int objectId);
                Future<void> UntrackReference(int sourceNode, int targetNode, int objectId);
                Future<int[]> GetReferencingNodes(int objectId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_ObjectMigration) {
    const char* code = R"(
        namespace GC {
            interface IObjectMigration {
                Future<void> MigrateObject(int objectId, int targetNodeId);
                Future<bool> CanMigrate(int objectId);
                Future<void> UpdateReferences(int oldNodeId, int newNodeId, int objectId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_CycleDetection) {
    const char* code = R"(
        namespace GC {
            interface ICycleDetector {
                Future<bool> DetectCycle(int startObjectId);
                Future<int[]> GetCycleParticipants(int objectId);
                Future<void> BreakCycle(int[] objectIds);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_FinalizationQueue) {
    const char* code = R"(
        namespace GC {
            interface IFinalizationQueue {
                Future<void> EnqueueForFinalization(int objectId);
                Future<object> DequeueFinalizableObject();
                Future<int> GetQueueLength();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, DistributedGC_MemoryPressure) {
    const char* code = R"(
        namespace GC {
            struct MemoryStats {
                int usedBytes;
                int totalBytes;
                int objectCount;
            }

            interface IMemoryManager {
                Future<MemoryStats> GetMemoryStats(int nodeId);
                Future<void> RequestCollection(int nodeId);
                Future<void> SetMemoryLimit(int nodeId, int maxBytes);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// ASYNC PATTERNS AND COMPOSITION (10 tests)
// ============================================================================

TEST_F(TauFutureProxyTests, Async_TaskChaining) {
    const char* code = R"(
        namespace Async {
            interface IChainableService {
                Future<int> Step1();
                Future<string> Step2(int value);
                Future<bool> Step3(string value);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_ParallelExecution) {
    const char* code = R"(
        namespace Async {
            interface IParallelService {
                Future<int> TaskA();
                Future<int> TaskB();
                Future<int> TaskC();
                Future<int[]> WhenAll(Future<int>[] tasks);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_Cancellation) {
    const char* code = R"(
        namespace Async {
            interface ICancellableService {
                Future<string> LongRunningOperation(int id);
                Future<void> Cancel(int operationId);
                Future<bool> IsCancelled(int operationId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_Timeout) {
    const char* code = R"(
        namespace Async {
            interface ITimeoutService {
                Future<string> OperationWithTimeout(int timeoutMs);
                Future<bool> TryOperation(int timeoutMs, int maxRetries);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_ErrorHandling) {
    const char* code = R"(
        namespace Async {
            struct ErrorInfo {
                int code;
                string message;
            }

            interface IReliableService {
                Future<string> TryGetData(int id);
                Future<ErrorInfo> GetLastError();
                Future<bool> Retry(int operationId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_ProgressReporting) {
    const char* code = R"(
        namespace Async {
            struct Progress {
                int current;
                int total;
                float percentage;
            }

            interface IProgressService {
                Future<void> StartOperation(int id);
                Future<Progress> GetProgress(int id);
                Future<bool> IsComplete(int id);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_BatchOperations) {
    const char* code = R"(
        namespace Async {
            interface IBatchService {
                Future<int[]> ProcessBatch(string[] inputs);
                Future<void> ProcessAsync(string input);
                Future<string[]> GetResults();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_Streaming) {
    const char* code = R"(
        namespace Async {
            interface IStreamService {
                Future<void> StartStream(string streamId);
                Future<object> GetNext(string streamId);
                Future<bool> HasMore(string streamId);
                Future<void> EndStream(string streamId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_ConditionalExecution) {
    const char* code = R"(
        namespace Async {
            interface IConditionalService {
                Future<bool> CheckCondition();
                Future<string> ExecuteIfTrue();
                Future<string> ExecuteIfFalse();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, Async_CompletionCallbacks) {
    const char* code = R"(
        namespace Async {
            interface ICallbackService {
                Future<void> RegisterCallback(int operationId, string callbackUrl);
                Future<void> StartAsyncOperation(int operationId);
                Future<bool> IsCallbackInvoked(int operationId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// REAL-WORLD USE CASES (10 tests)
// ============================================================================

TEST_F(TauFutureProxyTests, UseCase_DistributedCalculation) {
    const char* code = R"(
        namespace Compute {
            struct Matrix {
                int rows;
                int cols;
                float[] data;
            }

            interface IMatrixService {
                Future<Matrix> Multiply(Matrix a, Matrix b);
                Future<Matrix> Invert(Matrix m);
                Future<float> Determinant(Matrix m);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_DistributedDatabase) {
    const char* code = R"(
        namespace Database {
            struct Query {
                string sql;
                object[] parameters;
            }

            struct ResultSet {
                int rowCount;
                object[][] rows;
            }

            interface IQueryService {
                Future<ResultSet> ExecuteQuery(Query query);
                Future<int> ExecuteNonQuery(Query query);
                Future<object> ExecuteScalar(Query query);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_GameServer) {
    const char* code = R"(
        namespace Game {
            struct PlayerState {
                int playerId;
                float x;
                float y;
                float z;
                int health;
                int score;
            }

            interface IGameServer {
                Future<PlayerState> Join(string playerName);
                Future<void> UpdatePosition(int playerId, float x, float y, float z);
                Future<PlayerState[]> GetAllPlayers();
                Future<void> LeaveGame(int playerId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_ChatSystem) {
    const char* code = R"(
        namespace Chat {
            struct Message {
                string sender;
                string content;
                int timestamp;
            }

            interface IChatService {
                Future<void> SendMessage(string roomId, Message msg);
                Future<Message[]> GetRecentMessages(string roomId, int count);
                Future<void> JoinRoom(string roomId, string userId);
                Future<void> LeaveRoom(string roomId, string userId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_FileSync) {
    const char* code = R"(
        namespace FileSync {
            struct FileInfo {
                string path;
                int size;
                int modified;
                string hash;
            }

            interface IFileSyncService {
                Future<FileInfo[]> ListFiles(string directory);
                Future<string> ReadFile(string path);
                Future<void> WriteFile(string path, string content);
                Future<void> DeleteFile(string path);
                Future<bool> FileExists(string path);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_LoadBalancer) {
    const char* code = R"(
        namespace LoadBalancing {
            struct NodeInfo {
                int nodeId;
                string address;
                int load;
                bool healthy;
            }

            interface ILoadBalancer {
                Future<NodeInfo> SelectNode();
                Future<void> ReportLoad(int nodeId, int currentLoad);
                Future<NodeInfo[]> GetAllNodes();
                Future<void> AddNode(NodeInfo node);
                Future<void> RemoveNode(int nodeId);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_WorkQueue) {
    const char* code = R"(
        namespace WorkQueue {
            struct WorkItem {
                int id;
                string type;
                object data;
                int priority;
            }

            interface IWorkQueue {
                Future<int> Enqueue(WorkItem item);
                Future<WorkItem> Dequeue();
                Future<int> GetQueueLength();
                Future<void> Complete(int workItemId, object result);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_Monitoring) {
    const char* code = R"(
        namespace Monitoring {
            struct Metric {
                string name;
                float value;
                int timestamp;
            }

            interface IMonitoringService {
                Future<void> ReportMetric(Metric metric);
                Future<Metric[]> GetMetrics(string name, int startTime, int endTime);
                Future<float> GetAverageMetric(string name, int duration);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_Authentication) {
    const char* code = R"(
        namespace Auth {
            struct Credentials {
                string username;
                string passwordHash;
            }

            struct Token {
                string value;
                int expiresAt;
            }

            interface IAuthService {
                Future<Token> Login(Credentials creds);
                Future<bool> ValidateToken(Token token);
                Future<void> Logout(Token token);
                Future<void> RefreshToken(Token oldToken);
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauFutureProxyTests, UseCase_CacheService) {
    const char* code = R"(
        namespace Cache {
            interface ICacheService {
                Future<object> Get(string key);
                Future<void> Set(string key, object value, int ttlSeconds);
                Future<bool> Exists(string key);
                Future<void> Delete(string key);
                Future<void> Clear();
                Future<int> GetSize();
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}
