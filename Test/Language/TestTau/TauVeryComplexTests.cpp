#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"

using namespace kai;
using namespace std;

namespace {

string ComplexTradingTau() {
    return R"(
        namespace Trading { namespace Risk { namespace V2 {
            enum OrderSide { Buy, Sell }
            enum OrderState { New, Routed, Filled, Cancelled, Rejected }
            enum RiskDecision { Accept, Review, Reject }

            struct Money {
                float value;
                string currency;
            }

            struct Instrument {
                string symbol;
                string venue;
                int lotSize;
            }

            struct OrderLeg {
                Instrument instrument;
                OrderSide side;
                int quantity;
                Money limit;
            }

            struct RiskSignal {
                string name;
                float score;
                RiskDecision decision;
            }

            struct OrderRequest {
                string account;
                OrderLeg[] legs;
                RiskSignal[] signals;
                bool reduceOnly;
            }

            struct OrderAck {
                string orderId;
                OrderState state;
                RiskDecision decision;
                string[] warnings;
            }

            interface IRiskEngine {
                RiskDecision PreCheck(OrderRequest request);
                RiskSignal[] Explain(OrderRequest request, int maxSignals);
                bool IsVenueEnabled(string venue);
            }

            interface IOrderRouter {
                OrderAck Submit(OrderRequest request);
                OrderAck Cancel(string orderId, string reason);
                OrderState GetState(string orderId);
                async void SubscribeState(string account);
            }
        }}}
    )";
}

void ExpectAll(const string& output, const vector<string>& needles) {
    for (const auto& needle : needles) {
        EXPECT_NE(output.find(needle), string::npos)
            << "Missing '" << needle << "' in generated output:\n"
            << output;
    }
}

}  // namespace

TEST(TauVeryComplexTests, GeneratesProxyForDeepRiskAndRoutingModel) {
    string output;
    tau::Generate::GenerateProxy proxy(ComplexTradingTau().c_str(), output);

    ASSERT_FALSE(proxy.failed) << proxy.error;
    ASSERT_FALSE(output.empty());
    // Every proxy method returns a real Future<T> (via Exec<T> ->
    // Node::Invoke), so the declared return type is always wrapped.
    ExpectAll(output,
              {"namespace Trading", "namespace Risk", "namespace V2",
               "IRiskEngineProxy", "IOrderRouterProxy",
               "Future<RiskDecision> PreCheck(const OrderRequest& request)",
               "Explain(const OrderRequest& request",
               "Future<OrderAck> Submit(const OrderRequest& request)",
               "Future<void> SubscribeState(const string& account)"});
}

TEST(TauVeryComplexTests, GeneratesAgentForDeepRiskAndRoutingModel) {
    string output;
    tau::Generate::GenerateAgent agent(ComplexTradingTau().c_str(), output);

    ASSERT_FALSE(agent.failed) << agent.error;
    ASSERT_FALSE(output.empty());
    ExpectAll(output,
              {"namespace Trading", "namespace Risk", "namespace V2",
               "IRiskEngineAgent", "IOrderRouterAgent", "PreCheck",
               "Explain", "Submit", "Cancel", "SubscribeState"});
}
