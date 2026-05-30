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
        if (world.HostForRegion(agent.region_name) == host) {
            ++count;
        }
    }
    return count;
}

const MobileAgent* FindAgent(const World& world, int id) {
    for (const auto& agent : world.agents) {
        if (agent.id == id) return &agent;
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

    EXPECT_EQ(world.tick_count, 60);
    EXPECT_EQ(world.agents.size(), 10U);
    EXPECT_EQ(world.last_snapshot.size(), 10U);

    const std::set<int> expected_ids{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(AgentIds(world), expected_ids);
    EXPECT_EQ(CountAgentsOnHost(world, "NodeB"), 0);

    int restored_count = 0;
    for (const auto& agent : world.agents) {
        if (agent.restored) {
            ++restored_count;
            EXPECT_GE(agent.id, 1);
            EXPECT_LE(agent.id, 3);
            EXPECT_EQ(agent.region_name, "Backup");
            EXPECT_EQ(world.HostForRegion(agent.region_name), "NodeD");
        }
    }

    EXPECT_EQ(restored_count, 3);
}

TEST(ContinuationMobilityDemoTests, SnapshotCapturesCurrentAgentState) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    for (int i = 0; i < 20; ++i) {
        world.Tick();
    }

    testing::internal::GetCapturedStdout();

    ASSERT_EQ(world.last_snapshot.size(), world.agents.size());

    for (const auto& snapshot : world.last_snapshot) {
        const AgentState& state = snapshot.state;
        const MobileAgent* agent = FindAgent(world, state.id);
        ASSERT_NE(agent, nullptr);
        EXPECT_EQ(state.name, agent->name);
        EXPECT_EQ(state.region_name, agent->region_name);
        EXPECT_EQ(state.steps, agent->steps);
        EXPECT_EQ(state.plans_requested, agent->plans_requested);
        EXPECT_EQ(state.migrations, agent->migrations);
        EXPECT_EQ(state.restored, agent->restored);
    }
}

TEST(ContinuationMobilityDemoTests, PiReroutesMarketAgentsToHarborBeforeDrill) {
    World world = CreateDemoWorld();
    world.UpdateHostLoad();

    const MobileAgent* market_agent = FindAgent(world, 5);
    ASSERT_NE(market_agent, nullptr);

    Plan plan = world.pi.Ask(*market_agent, world);
    EXPECT_EQ(plan.summary, "market congestion detected; re-route to Harbor");
    ASSERT_TRUE(plan.target_region.has_value());
    EXPECT_EQ(*plan.target_region, "Harbor");
}

TEST(ContinuationMobilityDemoTests, PiHoldsMarketAgentsInPlaceDuringFailureDrill) {
    World world = CreateDemoWorld();
    world.tick_count = 58;
    world.UpdateHostLoad();

    MobileAgent drill_agent;
    drill_agent.id = 99;
    drill_agent.name = "DrillAgent";
    drill_agent.region_name = "Market";

    Plan plan = world.pi.Ask(drill_agent, world);
    EXPECT_EQ(plan.summary,
              "hold position in Market so failure recovery can be tested");
    EXPECT_FALSE(plan.target_region.has_value());
}

TEST(ContinuationMobilityDemoTests, BalanceLoadMovesAgentsToLeastLoadedHost) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    world.UpdateHostLoad();
    const int initial_min_load = world.host_load["NodeD"];
    world.BalanceLoad();

    testing::internal::GetCapturedStdout();

    world.UpdateHostLoad();
    EXPECT_EQ(world.host_load["NodeA"] + world.host_load["NodeB"] +
                  world.host_load["NodeC"] + world.host_load["NodeD"],
              10);
    EXPECT_EQ(world.host_load["NodeC"], 2);
    EXPECT_EQ(world.host_load["NodeD"], 2);
    EXPECT_GT(world.host_load["NodeD"], initial_min_load);
    EXPECT_EQ(CountAgentsOnHost(world, "NodeD"), 2);
    EXPECT_EQ(world.FindRegion("Backup")->visits, 2);

    const int max_load = std::max(
        std::max(world.host_load["NodeA"], world.host_load["NodeB"]),
        std::max(world.host_load["NodeC"], world.host_load["NodeD"]));
    const int min_load = std::min(
        std::min(world.host_load["NodeA"], world.host_load["NodeB"]),
        std::min(world.host_load["NodeC"], world.host_load["NodeD"]));
    EXPECT_LE(max_load - min_load, 2);
}

TEST(ContinuationMobilityDemoTests, FullRunPreservesAgentIdentityWithoutDuplication) {
    testing::internal::CaptureStdout();

    World world = CreateDemoWorld();
    for (int i = 0; i < 80; ++i) {
        world.Tick();
    }

    testing::internal::GetCapturedStdout();

    EXPECT_EQ(world.tick_count, 80);
    EXPECT_EQ(world.agents.size(), 10U);
    EXPECT_EQ(AgentIds(world).size(), world.agents.size());

    int restored_count = 0;
    for (const auto& agent : world.agents) {
        if (agent.restored) {
            ++restored_count;
        }
    }

    EXPECT_EQ(restored_count, 3);
    EXPECT_EQ(world.FindRegion("Market")->visits, 3);
    EXPECT_GE(world.FindRegion("Backup")->visits, restored_count);
}

}  // namespace
