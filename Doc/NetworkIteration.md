# Network Distributed Iteration in KAI

The AcrossAllNodes operation is a powerful feature in KAI that enables distributed parallel processing across networked nodes. This document explains the architecture, usage, and technical details of this feature.

## Architecture

AcrossAllNodes leverages KAI's distributed object model to enable parallel processing of collection elements across a network of nodes. The operation is implemented as part of the Executor system and is exposed through the Rho language interface.

### Components

- **Operation::AcrossAllNodes**: The operation enum value that represents the distributed iteration operation.
- **Executor::Perform(Operation::AcrossAllNodes)**: The implementation that handles the execution logic.
- **RhoLexer, RhoParser, RhoTranslator**: Language components that enable the `acrossAllNodes` keyword in Rho.
- **Network::Node**: The network node class that manages connections to peers.

### Execution Flow

1. **Parsing**: The Rho parser recognizes the `acrossAllNodes` keyword and translates it into an AcrossAllNodes operation.
2. **Argument Preparation**: Three arguments are prepared:
   - Network node (or null for local execution)
   - Collection to iterate over
   - Function to apply to each element
3. **Execution**:
   - If a null node is provided, execution falls back to local processing.
   - If a valid network node is provided, the operation distributes the workload across connected peers.
4. **Result Collection**: Results from all processing nodes are collected and combined into a single result collection.

## Usage in Rho

### Basic Syntax

```rho
result = acrossAllNodes(node, collection, function)
```

Where:
- `node` is a Network::Node object or null for local execution
- `collection` is a container like an Array, List, Map, or String
- `function` is the function to apply to each element

### Examples

#### Square all elements in an array

```rho
// Create a network node
node = createNetworkNode()
node.listen(14589)

// Connect to peers
node.connect("192.168.1.10", 14589)

// Define a function
fun square(x) { x * x }

// Create a test array
arr = [1, 2, 3, 4, 5]

// Process the array using network distribution
result = acrossAllNodes(node, arr, square)

// Output: [1, 4, 9, 16, 25]
print(result)
```

#### Process a map with a transform function

```rho
// Create a map
userData = {
  "alice": 25,
  "bob": 32,
  "charlie": 41
}

// Function to format user data
fun formatUser(pair) {
  name = pair[0]
  age = pair[1]
  return name + " is " + age + " years old"
}

// Process the map (locally in this case)
formattedData = acrossAllNodes(null, userData, formatUser)

// Print the results
for entry in formattedData
  print(entry)
end
```

#### Chained operations

```rho
// Define two transformation functions
fun double(x) { x * 2 }
fun addTen(x) { x + 10 }

// Create data
data = [1, 2, 3, 4, 5]

// Apply chained transformations
result = acrossAllNodes(node, 
           acrossAllNodes(node, data, double),
           addTen)

// Output: [12, 14, 16, 18, 20]
print(result)
```

## Performance Considerations

### Workload Balancing

For optimal performance, consider the following factors when using AcrossAllNodes:

1. **Computation vs. Communication Ratio**: The operation being performed should be computationally intensive enough to justify the network overhead. Simple operations may be faster when performed locally.

2. **Data Size**: Very large datasets benefit more from distribution than small ones.

3. **Network Latency**: High-latency networks may reduce the efficiency of distributed processing.

4. **Node Capacity**: The operation distributes work based on the number and capacity of connected peers.

### Benchmarking Example

```rho
// Create a large dataset
data = array(10000)
for i = 0; i < 10000; i = i + 1
  data[i] = i
end

// Define a computationally intensive operation
fun complexCalculation(x) {
  result = 0
  for j = 0; j < 1000; j = j + 1
    result = result + Math.sin(x * j) * Math.cos(j)
  end
  return result
}

// Time local execution
startLocal = currentTimeMillis()
localResult = acrossAllNodes(null, data, complexCalculation)
endLocal = currentTimeMillis()
localTime = endLocal - startLocal

// Time distributed execution
startDistributed = currentTimeMillis()
distributedResult = acrossAllNodes(node, data, complexCalculation)
endDistributed = currentTimeMillis()
distributedTime = endDistributed - startDistributed

// Compare results
print("Local execution time: " + localTime + "ms")
print("Distributed execution time: " + distributedTime + "ms")
print("Speedup factor: " + (localTime / distributedTime))
```

## Technical Implementation

The AcrossAllNodes operation is implemented in the Executor class and handles different collection types:

- **Array**: Elements are distributed and processed in parallel.
- **List**: Items are extracted and distributed to network nodes.
- **Map**: Key-value pairs are processed as Pair objects.
- **String**: Characters are processed individually.

The operation ensures type consistency between input and output collections and handles error cases appropriately.

### Code Structure

The implementation includes:

1. **Operation Enum**: Defined in Operation.h
   ```cpp
   enum Type {
       // ...
       AcrossAllNodes,
       // ...
   };
   ```

2. **Execution Logic**: Implemented in Executor.cpp
   ```cpp
   case Operation::AcrossAllNodes: {
       // Handle AcrossAllNodes operation
       // ...
   }
   ```

3. **Language Support**: In RhoLexer.cpp, RhoParser.cpp, RhoTranslator.cpp
   ```cpp
   // In RhoLexer.cpp
   keyWords["acrossAllNodes"] = Enum::AcrossAllNodes;

   // In RhoParser.cpp
   bool RhoParser::AcrossAllNodes(AstNodePtr block) {
       // Parse acrossAllNodes statement
       // ...
   }

   // In RhoTranslator.cpp
   void RhoTranslator::TranslateAcrossAllNodes(AstNodePtr node) {
       // Translate to bytecode
       // ...
   }
   ```

## Network Protocol

When executed with a valid network node, AcrossAllNodes uses the following protocol:

1. **Task Distribution**: The coordinator node serializes the function and data chunks.
2. **Work Assignment**: Data is distributed to peers based on their reported capacity.
3. **Processing**: Each peer processes its assigned chunk using the provided function.
4. **Result Collection**: Results are sent back to the coordinator.
5. **Aggregation**: The coordinator combines all results into the final collection.

This protocol minimizes network traffic while maximizing parallel processing capabilities.

## Error Handling

AcrossAllNodes includes error handling for various scenarios:

- **Node Failures**: If a node becomes unresponsive, its work is redistributed.
- **Type Errors**: Appropriate error messages are generated for incompatible types.
- **Function Errors**: Exceptions in the processing function are caught and reported.

## Future Enhancements

Planned enhancements for the AcrossAllNodes feature include:

1. **Dynamic Load Balancing**: Adjust distribution based on real-time node performance.
2. **Streaming Results**: Return partial results as they become available.
3. **Fault Tolerance**: More sophisticated handling of node failures.
4. **Security**: Encryption and authentication for distributed processing.
5. **More Collection Types**: Support for additional collection types like Set.

## Conclusion

The AcrossAllNodes operation represents a powerful fusion of KAI's object model and network capabilities, enabling truly distributed computation with minimal code complexity. By leveraging this feature, developers can easily scale processing across multiple nodes without having to manage the complexities of network communication, serialization, and workload distribution manually.