// Reference executable model for the continuation mobility story.
// The canonical version of the demo now lives in
// ContinuationMobilityDemo/ContinuationMobilityDemo.rho.

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
    std::optional<std::string> target_region;
};

struct AgentState {
    int id = 0;
    std::string name;
    std::string region_name;
    int steps = 0;
    int plans_requested = 0;
    int migrations = 0;
    bool restored = false;
};

struct AgentSnapshot {
    AgentState state;
};

struct MobileAgent : AgentState {
    AgentSnapshot Snapshot() const {
        return AgentSnapshot{static_cast<const AgentState&>(*this)};
    }
};

struct World;

struct PiAssistant {
    std::string host = "NodeC";

    Plan Ask(const MobileAgent& agent, const World& world) const;
};

struct World {
    std::map<std::string, Region> regions;
    std::vector<MobileAgent> agents;
    std::map<std::string, int> host_load;
    std::vector<AgentSnapshot> last_snapshot;
    PiAssistant pi;
    int tick_count = 0;

    void Log(const std::string& message) const {
        std::cout << "[tick " << std::setw(3) << tick_count << "] " << message
                  << '\n';
    }

    const Region* FindRegion(const std::string& name) const {
        auto it = regions.find(name);
        return it == regions.end() ? nullptr : &it->second;
    }

    Region* FindRegion(const std::string& name) {
        auto it = regions.find(name);
        return it == regions.end() ? nullptr : &it->second;
    }

    std::string HostForRegion(const std::string& region_name) const {
        const Region* region = FindRegion(region_name);
        return region ? region->host : "Unknown";
    }

    void UpdateHostLoad() {
        host_load.clear();
        for (const auto& [name, region] : regions) {
            host_load.try_emplace(region.host, 0);
            (void)name;
        }

        for (const auto& agent : agents) {
            host_load[HostForRegion(agent.region_name)] += 1;
        }
    }

    std::string RegionOnHost(const std::string& host,
                             const std::string& fallback_region) const {
        for (const auto& [name, region] : regions) {
            if (region.host == host) return name;
        }
        return fallback_region;
    }

    void MoveAgent(MobileAgent& agent, const std::string& target_region,
                   const std::string& reason) {
        const std::string current_host = HostForRegion(agent.region_name);
        const std::string target_host = HostForRegion(target_region);
        if (target_host == "Unknown") {
            Log("MoveAgent aborted for " + agent.name + ": unknown target " +
                target_region);
            return;
        }

        if (agent.region_name == target_region) {
            Log(agent.name + " stays in " + target_region + " (" + reason +
                ")");
            return;
        }

        if (current_host != target_host) {
            ++agent.migrations;
            Log("Suspending " + agent.name + " on " + current_host +
                " and resuming on " + target_host + " (" + reason + ")");
        } else {
            Log(agent.name + " moves within " + target_host + " from " +
                agent.region_name + " to " + target_region + " (" + reason +
                ")");
        }

        agent.region_name = target_region;
        if (Region* region = FindRegion(target_region)) {
            region->visits += 1;
        }
    }

    void BalanceLoad() {
        UpdateHostLoad();
        if (host_load.size() < 2) return;

        auto max_it = std::max_element(
            host_load.begin(), host_load.end(),
            [](const auto& left, const auto& right) {
                return left.second < right.second;
            });
        auto min_it = std::min_element(
            host_load.begin(), host_load.end(),
            [](const auto& left, const auto& right) {
                return left.second < right.second;
            });

        if (max_it == host_load.end() || min_it == host_load.end()) return;
        if (max_it->second - min_it->second <= 1) return;

        const std::string target_region =
            RegionOnHost(min_it->first, "Backup");
        int to_move = (max_it->second - min_it->second) / 2;

        Log("Balancer moving " + std::to_string(to_move) + " agents from " +
            max_it->first + " to " + min_it->first);

        for (auto& agent : agents) {
            if (to_move <= 0) break;
            if (HostForRegion(agent.region_name) != max_it->first) continue;
            MoveAgent(agent, target_region, "load balancing");
            --to_move;
        }
    }

    void Snapshot() {
        last_snapshot.clear();
        last_snapshot.reserve(agents.size());
        for (const auto& agent : agents) {
            last_snapshot.push_back(agent.Snapshot());
        }
        Log("Snapshot stored for " + std::to_string(last_snapshot.size()) +
            " agents");
    }

    void SimulateFailure(const std::string& failed_host,
                         const std::string& recovery_host) {
        std::vector<int> removed_ids;
        std::vector<MobileAgent> survivors;
        survivors.reserve(agents.size());

        for (const auto& agent : agents) {
            if (HostForRegion(agent.region_name) == failed_host) {
                removed_ids.push_back(agent.id);
            } else {
                survivors.push_back(agent);
            }
        }

        agents = std::move(survivors);

        std::ostringstream ids;
        for (std::size_t i = 0; i < removed_ids.size(); ++i) {
            if (i) ids << ", ";
            ids << removed_ids[i];
        }

        Log("Simulated failure on " + failed_host + ", removed " +
            std::to_string(removed_ids.size()) + " agents [" + ids.str() +
            "]");

        RecoverAgentsToHost(failed_host, recovery_host);
    }

    void RecoverAgentsToHost(const std::string& failed_host,
                             const std::string& recovery_host) {
        std::unordered_map<int, bool> alive;
        for (const auto& agent : agents) alive[agent.id] = true;

        const std::string recovery_region =
            RegionOnHost(recovery_host, "Backup");
        int restored_count = 0;

        for (const auto& snapshot : last_snapshot) {
            const AgentState& state = snapshot.state;
            if (alive.contains(state.id)) continue;
            if (HostForRegion(state.region_name) != failed_host) continue;

            MobileAgent recovered;
            recovered.id = state.id;
            recovered.name = state.name;
            recovered.region_name = recovery_region;
            recovered.steps = state.steps;
            recovered.plans_requested = state.plans_requested;
            recovered.migrations = state.migrations + 1;
            recovered.restored = true;
            agents.push_back(recovered);
            ++restored_count;

            Log("Recovered " + recovered.name + " from snapshot onto " +
                recovery_host + " via region " + recovery_region);
        }

        Log("Recovery finished: " + std::to_string(restored_count) +
            " agents restored");
    }

    void StepAgent(MobileAgent& agent) {
        ++agent.steps;

        // Force a small drill onto NodeB shortly before the failure event so
        // recovery is exercised every run.
        if (tick_count == 56 && agent.id >= 1 && agent.id <= 3) {
            MoveAgent(agent, "Market", "scheduled failure drill");
            return;
        }

        if (agent.steps % 5 == 0) {
            ++agent.plans_requested;
            Plan plan = pi.Ask(agent, *this);
            Log(agent.name + " consults Pi on " + pi.host + ": " +
                plan.summary);
            if (plan.target_region) {
                MoveAgent(agent, *plan.target_region, "Pi plan");
                return;
            }
        }

        if ((tick_count + agent.id) % 11 == 0) {
            const std::string current_host = HostForRegion(agent.region_name);
            std::string next_region = agent.region_name;
            for (const auto& [name, region] : regions) {
                if (region.host != current_host) {
                    next_region = name;
                    break;
                }
            }
            MoveAgent(agent, next_region, "autonomous patrol");
        }
    }

    void Tick() {
        ++tick_count;
        UpdateHostLoad();

        if (tick_count % 10 == 0) {
            std::ostringstream load_line;
            load_line << "Host load";
            for (const auto& [host, load] : host_load) {
                load_line << ' ' << host << '=' << load;
            }
            Log(load_line.str());
        }

        for (auto& agent : agents) StepAgent(agent);

        if (tick_count % 20 == 0) Snapshot();
        if (tick_count % 25 == 0) BalanceLoad();
        if (tick_count == 60) SimulateFailure("NodeB", "NodeD");
    }

    void PrintSummary() const {
        std::cout << "\n=== Final Summary ===\n";
        std::cout << "Ticks: " << tick_count << '\n';
        std::cout << "Agents: " << agents.size() << '\n';
        for (const auto& agent : agents) {
            std::cout << "  - " << agent.name << " region=" << agent.region_name
                      << " host=" << HostForRegion(agent.region_name)
                      << " steps=" << agent.steps
                      << " plans=" << agent.plans_requested
                      << " migrations=" << agent.migrations
                      << " restored=" << (agent.restored ? "yes" : "no")
                      << '\n';
        }

        std::cout << "\nRegion visits:\n";
        for (const auto& [name, region] : regions) {
            std::cout << "  - " << name << " on " << region.host
                      << " visits=" << region.visits << '\n';
        }
    }
};

Plan PiAssistant::Ask(const MobileAgent& agent, const World& world) const {
    const std::string current_host = world.HostForRegion(agent.region_name);
    const auto load_it = world.host_load.find(current_host);
    const int current_load =
        load_it == world.host_load.end() ? 0 : load_it->second;

    if (agent.region_name == "Market" && world.tick_count >= 56 &&
        world.tick_count <= 60) {
        return Plan{
            "hold position in Market so failure recovery can be tested",
            std::nullopt};
    }

    if (agent.region_name == "Market") {
        return Plan{"market congestion detected; re-route to Harbor",
                    std::string("Harbor")};
    }

    if (current_load >= 4) {
        return Plan{"host is saturated; move to backup capacity",
                    world.RegionOnHost("NodeD", "Backup")};
    }

    return Plan{"continue local survey on " + agent.region_name, std::nullopt};
}

World CreateDemoWorld() {
    World world;
    world.regions["Start"] = Region{"Start", "NodeA"};
    world.regions["Market"] = Region{"Market", "NodeB"};
    world.regions["Harbor"] = Region{"Harbor", "NodeC"};
    world.regions["Backup"] = Region{"Backup", "NodeD"};

    for (int i = 0; i < 10; ++i) {
        MobileAgent agent;
        agent.id = i + 1;
        agent.name = "Agent_" + std::to_string(i + 1);
        agent.region_name = (i < 4) ? "Start" : (i < 8) ? "Market" : "Harbor";
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
