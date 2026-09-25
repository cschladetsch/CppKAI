// Reference executable model for the continuation mobility story.
// The canonical version of the demo now lives in
// Demo/ContinuationMobilityDemo/ContinuationMobilityDemo.rho.

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct Region {
    std::string name;
    std::string host;
    int visits = 0;
};

struct Plan {
    std::string summary;
    std::optional<std::string> targetRegion;
};

struct AgentState {
    int id = 0;
    std::string name;
    std::string regionName;
    int steps = 0;
    int plansRequested = 0;
    int migrations = 0;
    bool restored = false;
};

struct AgentSnapshot {
    AgentState state;
};

struct MobileAgent : AgentState {
    [[nodiscard]] AgentSnapshot Snapshot() const
    {
        return AgentSnapshot{static_cast<const AgentState&>(*this)};
    }
};

struct World;

struct PiAssistant {
    std::string host = "NodeC";

    [[nodiscard]] static Plan Ask(const MobileAgent& agent, const World& world);
};

struct World {
    std::map<std::string, Region> regions;
    std::vector<MobileAgent> agents;
    std::map<std::string, int> host_load;
    std::vector<AgentSnapshot> lastSnapshot;
    PiAssistant pi;
    int tickCount = 0;

    void Log(const std::string& message) const {
        std::cout << "[tick " << std::setw(3) << tickCount << "] " << message << '\n';
    }

    [[nodiscard]] const Region* FindRegion(const std::string& name) const
    {
        auto it = regions.find(name);
        return it == regions.end() ? nullptr : &it->second;
    }

    Region* FindRegion(const std::string& name) {
        auto it = regions.find(name);
        return it == regions.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::string HostForRegion(const std::string& regionName) const
    {
        const Region* region = FindRegion(regionName);
        return (region != nullptr) ? region->host : "Unknown";
    }

    void UpdateHostLoad() {
        host_load.clear();
        for (const auto& [name, region] : regions) {
            host_load.try_emplace(region.host, 0);
            (void)name;
        }

        for (const auto& agent : agents) {
            host_load[HostForRegion(agent.regionName)] += 1;
        }
    }

    [[nodiscard]] std::string RegionOnHost(const std::string& host, const std::string& fallbackRegion) const
    {
        for (const auto& [name, region] : regions) {
            if (region.host == host) {
                return name;
            }
        }
        return fallbackRegion;
    }

    void MoveAgent(MobileAgent& agent, const std::string& targetRegion, const std::string& reason)
    {
        const std::string currentHost = HostForRegion(agent.regionName);
        const std::string targetHost = HostForRegion(targetRegion);
        if (targetHost == "Unknown") {
            Log("MoveAgent aborted for " + agent.name + ": unknown target " + targetRegion);
            return;
        }

        if (agent.regionName == targetRegion) {
            Log(agent.name + " stays in " + targetRegion + " (" + reason + ")");
            return;
        }

        if (currentHost != targetHost) {
            ++agent.migrations;
            Log("Suspending " + agent.name + " on " + currentHost + " and resuming on " + targetHost + " (" + reason +
                ")");
        } else {
            Log(agent.name + " moves within " + targetHost + " from " + agent.regionName + " to " + targetRegion +
                " (" + reason + ")");
        }

        agent.regionName = targetRegion;
        if (Region* region = FindRegion(targetRegion)) {
            region->visits += 1;
        }
    }

    void BalanceLoad() {
        UpdateHostLoad();
        if (host_load.size() < 2) {
            return;
        }

        auto maxIt = std::ranges::max_element(
            host_load, [](const auto& left, const auto& right) { return left.second < right.second; });
        auto minIt = std::ranges::min_element(
            host_load, [](const auto& left, const auto& right) { return left.second < right.second; });

        if (maxIt == host_load.end() || minIt == host_load.end()) {
            return;
        }
        if (maxIt->second - minIt->second <= 1) {
            return;
        }

        const std::string targetRegion = RegionOnHost(minIt->first, "Backup");
        int toMove = (maxIt->second - minIt->second) / 2;

        Log("Balancer moving " + std::to_string(toMove) + " agents from " + maxIt->first + " to " + minIt->first);

        for (auto& agent : agents) {
            if (toMove <= 0) {
                break;
            }
            if (HostForRegion(agent.regionName) != maxIt->first) {
                continue;
            }
            MoveAgent(agent, targetRegion, "load balancing");
            --toMove;
        }
    }

    void Snapshot() {
        lastSnapshot.clear();
        lastSnapshot.reserve(agents.size());
        for (const auto& agent : agents) {
            lastSnapshot.push_back(agent.Snapshot());
        }
        Log("Snapshot stored for " + std::to_string(lastSnapshot.size()) + " agents");
    }

    void SimulateFailure(const std::string& failedHost, const std::string& recoveryHost)
    {
        std::vector<int> removedIds;
        std::vector<MobileAgent> survivors;
        survivors.reserve(agents.size());

        for (const auto& agent : agents) {
            if (HostForRegion(agent.regionName) == failedHost) {
                removedIds.push_back(agent.id);
            } else {
                survivors.push_back(agent);
            }
        }

        agents = std::move(survivors);

        std::ostringstream ids;
        for (std::size_t i = 0; i < removedIds.size(); ++i) {
            if (i != 0u) {
                ids << ", ";
            }
            ids << removedIds[i];
        }

        Log("Simulated failure on " + failedHost + ", removed " + std::to_string(removedIds.size()) + " agents [" +
            ids.str() + "]");

        RecoverAgentsToHost(failedHost, recoveryHost);
    }

    void RecoverAgentsToHost(const std::string& failedHost, const std::string& recoveryHost)
    {
        std::unordered_map<int, bool> alive;
        for (const auto& agent : agents) {
            alive[agent.id] = true;
        }

        const std::string recoveryRegion = RegionOnHost(recoveryHost, "Backup");
        int restoredCount = 0;

        for (const auto& snapshot : lastSnapshot) {
            const AgentState& state = snapshot.state;
            if (alive.contains(state.id)) {
                continue;
            }
            if (HostForRegion(state.regionName) != failedHost) {
                continue;
            }

            MobileAgent recovered;
            recovered.id = state.id;
            recovered.name = state.name;
            recovered.regionName = recoveryRegion;
            recovered.steps = state.steps;
            recovered.plansRequested = state.plansRequested;
            recovered.migrations = state.migrations + 1;
            recovered.restored = true;
            agents.push_back(recovered);
            ++restoredCount;

            Log("Recovered " + recovered.name + " from snapshot onto " + recoveryHost + " via region " +
                recoveryRegion);
        }

        Log("Recovery finished: " + std::to_string(restoredCount) + " agents restored");
    }

    void StepAgent(MobileAgent& agent) {
        ++agent.steps;

        // Force a small drill onto NodeB shortly before the failure event so
        // recovery is exercised every run.
        if (tickCount == 56 && agent.id >= 1 && agent.id <= 3) {
            MoveAgent(agent, "Market", "scheduled failure drill");
            return;
        }

        if (agent.steps % 5 == 0) {
            ++agent.plansRequested;
            Plan plan = pi.Ask(agent, *this);
            Log(agent.name + " consults Pi on " + pi.host + ": " +
                plan.summary);
            if (plan.targetRegion) {
                MoveAgent(agent, *plan.targetRegion, "Pi plan");
                return;
            }
        }

        if ((tickCount + agent.id) % 11 == 0) {
            const std::string currentHost = HostForRegion(agent.regionName);
            std::string nextRegion = agent.regionName;
            for (const auto& [name, region] : regions) {
                if (region.host != currentHost) {
                    nextRegion = name;
                    break;
                }
            }
            MoveAgent(agent, nextRegion, "autonomous patrol");
        }
    }

    void Tick() {
        ++tickCount;
        UpdateHostLoad();

        if (tickCount % 10 == 0) {
            std::ostringstream loadLine;
            loadLine << "Host load";
            for (const auto& [host, load] : host_load) {
                loadLine << ' ' << host << '=' << load;
            }
            Log(loadLine.str());
        }

        for (auto& agent : agents) {
            StepAgent(agent);
        }

        if (tickCount % 20 == 0) {
            Snapshot();
        }
        if (tickCount % 25 == 0) {
            BalanceLoad();
        }
        if (tickCount == 60) {
            SimulateFailure("NodeB", "NodeD");
        }
    }

    void PrintSummary() const {
        std::cout << "\n=== Final Summary ===\n";
        std::cout << "Ticks: " << tickCount << '\n';
        std::cout << "Agents: " << agents.size() << '\n';
        for (const auto& agent : agents) {
            std::cout << "  - " << agent.name << " region=" << agent.regionName
                      << " host=" << HostForRegion(agent.regionName) << " steps=" << agent.steps
                      << " plans=" << agent.plansRequested << " migrations=" << agent.migrations
                      << " restored=" << (agent.restored ? "yes" : "no") << '\n';
        }

        std::cout << "\nRegion visits:\n";
        for (const auto& [name, region] : regions) {
            std::cout << "  - " << name << " on " << region.host
                      << " visits=" << region.visits << '\n';
        }
    }
};

Plan PiAssistant::Ask(const MobileAgent& agent, const World& world)
{
    const std::string currentHost = world.HostForRegion(agent.regionName);
    const auto loadIt = world.host_load.find(currentHost);
    const int currentLoad = loadIt == world.host_load.end() ? 0 : loadIt->second;

    if (agent.regionName == "Market" && world.tickCount >= 56 && world.tickCount <= 60) {
        return Plan{.summary = "hold position in Market so failure recovery can be tested",
                    .targetRegion = std::nullopt};
    }

    if (agent.regionName == "Market") {
        return Plan{.summary = "market congestion detected; re-route to Harbor",
                    .targetRegion = std::string("Harbor")};
    }

    if (currentLoad >= 4) {
        return Plan{.summary = "host is saturated; move to backup capacity",
                    .targetRegion = world.RegionOnHost("NodeD", "Backup")};
    }

    return Plan{.summary = "continue local survey on " + agent.regionName, .targetRegion = std::nullopt};
}

World CreateDemoWorld() {
    World world;
    world.regions["Start"] = Region{.name = "Start", .host = "NodeA"};
    world.regions["Market"] = Region{.name = "Market", .host = "NodeB"};
    world.regions["Harbor"] = Region{.name = "Harbor", .host = "NodeC"};
    world.regions["Backup"] = Region{.name = "Backup", .host = "NodeD"};

    for (int i = 0; i < 10; ++i) {
        MobileAgent agent;
        agent.id = i + 1;
        agent.name = "Agent_" + std::to_string(i + 1);
        agent.regionName = (i < 4) ? "Start" : (i < 8) ? "Market" : "Harbor";
        world.agents.push_back(agent);
    }

    world.Log("World initialized with 4 regions and 10 mobile agents");
    return world;
}

}  // namespace

#ifndef KAI_DEMO1_NO_MAIN
int main() {
    World world = CreateDemoWorld();

    for (int i = 0; i < 80; ++i) {
        world.Tick();
    }

    world.PrintSummary();
    return 0;
}
#endif
