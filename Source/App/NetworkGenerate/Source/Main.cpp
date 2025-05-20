#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"

using namespace std;
using namespace kai;
using namespace boost::filesystem;
using namespace boost::program_options;

enum Generate {
    None = 0,
    Agent = 1,
    Proxy = 2,
    Both = Agent | Proxy,
};

int main(int argc, const char* const argv[]) {
    options_description desc("Options");

    path out_dir = ".";
    path agent_out_dir = ".";
    path proxy_out_dir = ".";
    string agentOutputDir = ".";
    string proxyOutputDir = ".";

    string proxyName = "%s.proxy.h";
    string agentName = "%s.agent.h";

    desc.add_options()("help", "Talk to Christian")("input", value<path>(),
                                                    "Input TAU file")(
        "proxy_dir", value<path>(&proxy_out_dir)->default_value(proxyOutputDir),
        "Set output dir for proxy")(
        "agent_dir", value<path>(&agent_out_dir)->default_value(agentOutputDir),
        "Set output dir for agent")(
        "proxy_name", value<string>(&proxyName)->default_value("%s.proxy.h"),
        "Set output name for proxy")(
        "agent_name", value<string>(&agentName)->default_value("%s.agent.h"),
        "Set output name for agent")("out",
                                     value<path>(&out_dir)->default_value("."),
                                     "Set output dir for both agent and proxy")(
        "start", value<path>(), "File included before anything else")(
        "pre", value<path>(),
        "File included after start and before definition")(
        "post", value<path>(), "File included after definition");

    positional_options_description p;
    p.add("input", -1);

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).positional(p).run(),
          vm);
    notify(vm);

    if (vm.count("input") != 1) {
        cerr << desc;
        return 1;
    }

    auto input = vm["input"].as<path>();

    // Get the filename without extension
    string filename = input.stem().string();

    // Format the output filenames using the provided format strings
    string formattedProxyName = proxyName;
    string formattedAgentName = agentName;

    // Replace %s with the filename
    size_t pos = formattedProxyName.find("%s");
    if (pos != string::npos) {
        formattedProxyName.replace(pos, 2, filename);
    }

    pos = formattedAgentName.find("%s");
    if (pos != string::npos) {
        formattedAgentName.replace(pos, 2, filename);
    }

    // Construct full output paths
    path outputProxyPath = vm.count("out") ? out_dir / formattedProxyName
                                           : proxy_out_dir / formattedProxyName;
    path outputAgentPath = vm.count("out") ? out_dir / formattedAgentName
                                           : agent_out_dir / formattedAgentName;

    // Convert to string for use in the generator functions
    string outputProxy = outputProxyPath.string();
    string outputAgent = outputAgentPath.string();

    // Use the input file path and the string output paths
    cout << "Input file: " << input << endl;
    cout << "Output proxy path: " << outputProxy << endl;
    cout << "Output agent path: " << outputAgent << endl;

    if (!outputProxy.empty()) {
        cout << "Generating proxy..." << endl;
        // Convert input path to string and use that for the input file
        string inputStr = input.string();
        string proxyOutput;
        tau::Generate::GenerateProxy proxy(inputStr.c_str(), proxyOutput);
        if (proxy.Failed) {
            cerr << "ProxyGenError: " << proxy.Error << endl;
            return 1;
        }

        // Write output to file
        ofstream proxyFile(outputProxy);
        if (proxyFile) {
            proxyFile << proxyOutput;
            proxyFile.close();
            cout << "Wrote proxy code to " << outputProxy << endl;
        } else {
            cerr << "Error: Could not open " << outputProxy << " for writing"
                 << endl;
            return 1;
        }
    }

    if (!outputAgent.empty()) {
        cout << "Generating agent..." << endl;
        // Convert input path to string and use that for the input file
        string inputStr = input.string();
        string agentOutput;
        tau::Generate::GenerateAgent agent(inputStr.c_str(), agentOutput);
        if (agent.Failed) {
            cerr << "AgentGenError: " << agent.Error << endl;
            return 1;
        }

        // Write output to file
        ofstream agentFile(outputAgent);
        if (agentFile) {
            agentFile << agentOutput;
            agentFile.close();
            cout << "Wrote agent code to " << outputAgent << endl;
        } else {
            cerr << "Error: Could not open " << outputAgent << " for writing"
                 << endl;
            return 1;
        }
    }

    return 0;
}
