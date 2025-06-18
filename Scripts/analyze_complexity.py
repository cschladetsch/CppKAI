#!/usr/bin/env python3
"""
Cyclomatic Complexity Analysis Script for KAI Repository

This script analyzes the cyclomatic complexity of the codebase using lizard.
It provides detailed metrics and identifies functions that exceed complexity thresholds.
"""

import subprocess
import sys
import json
import argparse
from pathlib import Path
import os

# Default configuration
DEFAULT_CONFIG = {
    "warning_threshold": 10,    # Functions with CCN > 10 are warnings
    "error_threshold": 25,      # Functions with CCN > 25 are errors
    "max_function_length": 100, # Functions longer than this are warnings
    "exclude_patterns": [
        "*/Build/*",
        "*/ThirdParty/*",
        "*/External/*",
        "*/Test/*",  # Can be included with --include-tests
        "*/.git/*"
    ],
    "file_extensions": [".cpp", ".h", ".hpp", ".c", ".cc"]
}

def check_lizard_installed():
    """Check if lizard is installed."""
    try:
        subprocess.run(["lizard", "--version"], capture_output=True, check=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

def install_lizard():
    """Install lizard using pip."""
    print("Installing lizard...")
    try:
        subprocess.run([sys.executable, "-m", "pip", "install", "lizard"], check=True)
        print("Lizard installed successfully!")
        return True
    except subprocess.CalledProcessError:
        print("Failed to install lizard. Please install it manually: pip install lizard")
        return False

def run_complexity_analysis(paths, config, include_tests=False, output_format="console"):
    """Run lizard complexity analysis on the specified paths."""
    cmd = ["lizard"]
    
    # Add warning threshold
    cmd.extend(["-w", str(config["warning_threshold"])])
    
    # Add function length threshold
    cmd.extend(["-L", str(config["max_function_length"])])
    
    # Add exclude patterns
    exclude_patterns = config["exclude_patterns"].copy()
    if include_tests:
        exclude_patterns = [p for p in exclude_patterns if "Test" not in p]
    
    for pattern in exclude_patterns:
        cmd.extend(["-x", pattern])
    
    # Add file extensions
    extensions = "*" + " *".join(config["file_extensions"])
    cmd.extend(["-l", "cpp"])
    
    # Add output format
    if output_format == "json":
        cmd.append("-o")
        cmd.append("json")
    elif output_format == "csv":
        cmd.append("-o")
        cmd.append("csv")
    
    # Add paths
    cmd.extend(paths)
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return result.stdout, result.stderr
    except subprocess.CalledProcessError as e:
        return e.stdout, e.stderr

def parse_console_output(output):
    """Parse the console output to extract summary statistics."""
    lines = output.strip().split('\n')
    summary = {}
    
    for line in lines:
        if "Total nloc" in line:
            parts = line.split()
            if len(parts) >= 2:
                summary["total_nloc"] = int(parts[2]) if parts[2].isdigit() else 0
        elif "Average cyclomatic complexity" in line:
            parts = line.split(':')
            if len(parts) >= 2:
                try:
                    summary["average_complexity"] = float(parts[1].strip())
                except ValueError:
                    summary["average_complexity"] = 0.0
        elif "Average token" in line:
            parts = line.split(':')
            if len(parts) >= 2:
                try:
                    summary["average_tokens"] = float(parts[1].strip())
                except ValueError:
                    summary["average_tokens"] = 0.0
    
    return summary

def find_high_complexity_functions(output, error_threshold):
    """Extract functions with high complexity from the output."""
    high_complexity = []
    lines = output.strip().split('\n')
    
    in_function_list = False
    for line in lines:
        if line.startswith("="):
            in_function_list = True
            continue
        
        if in_function_list and line.strip() and not line.startswith("Total"):
            parts = line.split()
            if len(parts) >= 5:
                try:
                    nloc = int(parts[0])
                    ccn = int(parts[1])
                    token = int(parts[2])
                    param = int(parts[3])
                    function_name = ' '.join(parts[4:])
                    
                    if ccn >= error_threshold:
                        high_complexity.append({
                            "function": function_name,
                            "complexity": ccn,
                            "lines": nloc,
                            "tokens": token,
                            "parameters": param
                        })
                except (ValueError, IndexError):
                    continue
    
    return sorted(high_complexity, key=lambda x: x["complexity"], reverse=True)

def main():
    parser = argparse.ArgumentParser(description="Analyze cyclomatic complexity of KAI codebase")
    parser.add_argument("paths", nargs="*", default=["."], help="Paths to analyze (default: current directory)")
    parser.add_argument("--include-tests", action="store_true", help="Include test files in analysis")
    parser.add_argument("--warning-threshold", type=int, default=DEFAULT_CONFIG["warning_threshold"],
                        help=f"Complexity warning threshold (default: {DEFAULT_CONFIG['warning_threshold']})")
    parser.add_argument("--error-threshold", type=int, default=DEFAULT_CONFIG["error_threshold"],
                        help=f"Complexity error threshold (default: {DEFAULT_CONFIG['error_threshold']})")
    parser.add_argument("--output", choices=["console", "json", "csv"], default="console",
                        help="Output format (default: console)")
    parser.add_argument("--config", help="Path to configuration file")
    parser.add_argument("--check-only", action="store_true", 
                        help="Exit with error code if any function exceeds error threshold")
    
    args = parser.parse_args()
    
    # Load configuration
    config = DEFAULT_CONFIG.copy()
    if args.config and Path(args.config).exists():
        with open(args.config, 'r') as f:
            user_config = json.load(f)
            config.update(user_config)
    
    # Override with command line arguments
    config["warning_threshold"] = args.warning_threshold
    config["error_threshold"] = args.error_threshold
    
    # Check if lizard is installed
    if not check_lizard_installed():
        if not install_lizard():
            sys.exit(1)
    
    # Run analysis
    print(f"Analyzing cyclomatic complexity...")
    print(f"Warning threshold: {config['warning_threshold']}")
    print(f"Error threshold: {config['error_threshold']}")
    print(f"Include tests: {args.include_tests}")
    print()
    
    stdout, stderr = run_complexity_analysis(args.paths, config, args.include_tests, args.output)
    
    if args.output == "console":
        print(stdout)
        
        # Parse and display summary
        summary = parse_console_output(stdout)
        if summary:
            print("\n" + "="*80)
            print("SUMMARY:")
            print(f"Average Cyclomatic Complexity: {summary.get('average_complexity', 'N/A')}")
            print(f"Total Lines of Code: {summary.get('total_nloc', 'N/A')}")
            
        # Find and display high complexity functions
        high_complexity = find_high_complexity_functions(stdout, config["error_threshold"])
        if high_complexity:
            print(f"\nFUNCTIONS EXCEEDING ERROR THRESHOLD ({config['error_threshold']}):")
            print("-" * 80)
            for func in high_complexity[:10]:  # Show top 10
                print(f"CCN: {func['complexity']:3d} | Lines: {func['lines']:4d} | {func['function']}")
            
            if len(high_complexity) > 10:
                print(f"\n... and {len(high_complexity) - 10} more functions")
            
            if args.check_only:
                print(f"\nERROR: {len(high_complexity)} functions exceed complexity threshold!")
                sys.exit(1)
        else:
            print(f"\nNo functions exceed the error threshold ({config['error_threshold']})")
            
    else:
        # For JSON/CSV output, just print the raw output
        print(stdout)
    
    if stderr:
        print("\nWarnings/Errors:", file=sys.stderr)
        print(stderr, file=sys.stderr)

if __name__ == "__main__":
    main()