#include <algorithm>
#include <set>

#include <gtest/gtest.h>

#define KAI_DEMO1_NO_MAIN
#include "../../Demo/ContinuationMobilityDemo/ContinuationMobilityDemo.cpp"

namespace {

std::set<int> AgentIds(const World& world) {
    std::set<int> ids;
    for (const auto& agent : world.agents) {
        ids.insert(agent.id);
    }
    return ids;
}

int CountAgentsOnHost(const World& world, const std::string& host) {
    int count = 0;
    for (const auto& agent : world.agents) {
        if (world.HostForRegion(agent.regionName) == host) {
            ++count;
        }
    }
    return count;
}

const MobileAgent* FindAgent(const World& world, int id) {
    for (const auto& agent : world.agents) {
        if (agent.id == id) {
            return &agent;
        }
    }
    return nullptr;
}

TEST(ContinuationMobilityDemoTests, InitialWorldHasExpectedTopology) {
    World world = CreateDemoWorld();

    EXPECT_EQ(world.regions.size(), 4U);
    EXPECT_EQ(world.agents.size(), 10U);
    EXPECT_EQ(world.HostForRegion("Start"), "NodeA");
    EXPECT_EQ(world.HostForRegion("Market"), "NodeB");
    EXPECT_EQ(world.HostForRegion("Harbor"), "NodeC");
    EXPECT_EQ(world.HostForRegion("Backup"), "NodeD");

    world.UpdateHostLoad();
    EXPECT_EQ(world.host_load["NodeA"], 4);
    EXPECT_EQ(world.host_load["NodeB"], 4);
    EXPECT_EQ(world.host_load["NodeC"], 2);
    EXPECT_EQ(world.host_load["NodeD"], 0);
}

TEST(ContinuationMobilityDemoTests, FailureDrillRemovesAndRestoresThreeAgents) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    for (int i = 0; i < 60; ++i) {
        world.Tick();
    }

    testing::internal::GetCapturedStdout();

    EXPECT_EQ(world.tickCount, 60);
    EXPECT_EQ(world.agents.size(), 10U);
    EXPECT_EQ(world.lastSnapshot.size(), 10U);

    const std::set<int> expectedIds{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(AgentIds(world), expectedIds);
    EXPECT_EQ(CountAgentsOnHost(world, "NodeB"), 0);

    int restoredCount = 0;
    for (const auto& agent : world.agents) {
        if (agent.restored) {
            ++restoredCount;
            EXPECT_GE(agent.id, 1);
            EXPECT_LE(agent.id, 3);
            EXPECT_EQ(agent.regionName, "Backup");
            EXPECT_EQ(world.HostForRegion(agent.regionName), "NodeD");
        }
    }

    EXPECT_EQ(restoredCount, 3);
}

TEST(ContinuationMobilityDemoTests, SnapshotCapturesCurrentAgentState) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    for (int i = 0; i < 20; ++i) {
        world.Tick();
    }

    testing::internal::GetCapturedStdout();

    ASSERT_EQ(world.lastSnapshot.size(), world.agents.size());

    for (const auto& snapshot : world.lastSnapshot) {
        const AgentState& state = snapshot.state;
        const MobileAgent* agent = FindAgent(world, state.id);
        ASSERT_NE(agent, nullptr);
        EXPECT_EQ(state.name, agent->name);
        EXPECT_EQ(state.regionName, agent->regionName);
        EXPECT_EQ(state.steps, agent->steps);
        EXPECT_EQ(state.plansRequested, agent->plansRequested);
        EXPECT_EQ(state.migrations, agent->migrations);
        EXPECT_EQ(state.restored, agent->restored);
    }
}

TEST(ContinuationMobilityDemoTests, PiReroutesMarketAgentsToHarborBeforeDrill) {
    World world = CreateDemoWorld();
    world.UpdateHostLoad();

    const MobileAgent* marketAgent = FindAgent(world, 5);
    ASSERT_NE(marketAgent, nullptr);

    Plan plan = world.pi.Ask(*marketAgent, world);
    EXPECT_EQ(plan.summary, "market congestion detected; re-route to Harbor");
    ASSERT_TRUE(plan.targetRegion.has_value());
    EXPECT_EQ(*plan.targetRegion, "Harbor");
}

TEST(ContinuationMobilityDemoTests, PiHoldsMarketAgentsInPlaceDuringFailureDrill) {
    World world = CreateDemoWorld();
    world.tickCount = 58;
    world.UpdateHostLoad();

    MobileAgent drillAgent;
    drillAgent.id = 99;
    drillAgent.name = "DrillAgent";
    drillAgent.regionName = "Market";

    Plan plan = world.pi.Ask(drillAgent, world);
    EXPECT_EQ(plan.summary,
              "hold position in Market so failure recovery can be tested");
    EXPECT_FALSE(plan.targetRegion.has_value());
}

TEST(ContinuationMobilityDemoTests, BalanceLoadMovesAgentsToLeastLoadedHost) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    world.UpdateHostLoad();
    const int initialMinLoad = world.host_load["NodeD"];
    world.BalanceLoad();

    testing::internal::GetCapturedStdout();

    world.UpdateHostLoad();
    EXPECT_EQ(world.host_load["NodeA"] + world.host_load["NodeB"] +
                  world.host_load["NodeC"] + world.host_load["NodeD"],
              10);
    EXPECT_EQ(world.host_load["NodeC"], 2);
    EXPECT_EQ(world.host_load["NodeD"], 2);
    EXPECT_GT(world.host_load["NodeD"], initialMinLoad);
    EXPECT_EQ(CountAgentsOnHost(world, "NodeD"), 2);
    EXPECT_EQ(world.FindRegion("Backup")->visits, 2);

    const int maxLoad = std::max(
        {world.host_load["NodeA"], world.host_load["NodeB"], world.host_load["NodeC"], world.host_load["NodeD"]});
    const int minLoad = std::min(
        {world.host_load["NodeA"], world.host_load["NodeB"], world.host_load["NodeC"], world.host_load["NodeD"]});
    EXPECT_LE(maxLoad - minLoad, 2);
}

TEST(ContinuationMobilityDemoTests, FullRunPreservesAgentIdentityWithoutDuplication) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    for (int i = 0; i < 80; ++i) {
        world.Tick();
    }

    testing::internal::GetCapturedStdout();

    EXPECT_EQ(world.tickCount, 80);
    EXPECT_EQ(world.agents.size(), 10U);
    EXPECT_EQ(AgentIds(world).size(), world.agents.size());

    int restoredCount = 0;
    for (const auto& agent : world.agents) {
        if (agent.restored) {
            ++restoredCount;
        }
    }

    EXPECT_EQ(restoredCount, 3);
    EXPECT_EQ(world.FindRegion("Market")->visits, 3);
    EXPECT_GE(world.FindRegion("Backup")->visits, restoredCount);
}

}  // namespace
