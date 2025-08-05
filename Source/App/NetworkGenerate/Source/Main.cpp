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

    desc.add_options()("help,h", "Show this help message")
                      ("version,v", "Show version information")
                      ("input,i", value<path>(), "Input TAU file (required)")(
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
    try {
        store(command_line_parser(argc, argv).options(desc).positional(p).run(),
              vm);
        notify(vm);
    } catch (const exception& e) {
        cerr << "Error parsing command line: " << e.what() << endl;
        return 1;
    }

    if (vm.count("help")) {
        cout << "NetworkGenerate - Tau IDL Code Generator" << endl;
        cout << "Generates C++ proxy and agent classes from Tau interface definitions" << endl << endl;
        cout << "Usage: NetworkGenerate [options] <input.tau>" << endl << endl;
        cout << desc << endl;
        cout << "Examples:" << endl;
        cout << "  NetworkGenerate Calculator.tau" << endl;
        cout << "  NetworkGenerate Calculator.tau --out=./generated" << endl;
        cout << "  NetworkGenerate Calculator.tau --proxy_dir=./client --agent_dir=./server" << endl << endl;
        cout << "For more information, see the NetworkGenerate README.md" << endl;
        return 0;
    }

    if (vm.count("version")) {
        cout << "NetworkGenerate version 1.0.0" << endl;
        cout << "Part of the KAI distributed object model system" << endl;
        cout << "Built with Boost " << BOOST_VERSION / 100000 << "." 
             << BOOST_VERSION / 100 % 1000 << "." << BOOST_VERSION % 100 << endl;
        return 0;
    }

    if (vm.count("input") != 1) {
        cerr << "Error: Input TAU file is required" << endl << endl;
        cout << "Usage: NetworkGenerate [options] <input.tau>" << endl;
        cout << "Use --help for more information" << endl;
        return 1;
    }

    auto input = vm["input"].as<path>();

    // Validate input file exists
    if (!exists(input)) {
        cerr << "Error: Input file '" << input << "' does not exist" << endl;
        return 1;
    }

    // Validate input file extension
    if (input.extension() != ".tau") {
        cout << "Warning: Input file does not have .tau extension. Proceeding anyway..." << endl;
    }

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
