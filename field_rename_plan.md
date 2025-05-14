# Field Renaming Plan: Prefix to Postfix Underscore

## Approach
1. Identify all class members with prefix underscores (_fieldName)
2. Create a mapping of old names to new names (fieldName_)
3. Create search and replace patterns for each file
4. Apply changes systematically, file by file
5. Test after each major component is updated

## Field Naming Patterns Found
Based on the code review, we found these patterns of field naming:

1. In Object.h:
   - `_gcIndex` → `gcIndex_`
   - `_valid` → `valid_`

2. In Executor.h:
   - `_continuation` → `continuation_`
   - `_context` → `context_`
   - `_data` → `data_`
   - `_compiler` → `compiler_`
   - `_break` → `break_`
   - `_tree` → `tree_`
   - `_traceLevel` → `traceLevel_`
   - `_stepNumber` → `stepNumber_`

3. In Registry.h and other classes:
   - All member variables with underscore prefix will follow the same pattern

## Implementation Steps

### 1. Create a search tool
- Write a script that can find all class member variables with underscore prefix
- Generate a mapping of old → new names

### 2. Define Update Batches
- Core library (Object, Registry, etc.)
- Executor library
- Language modules (Pi, Rho, etc.)

### 3. For Each Batch:
- Update the header (.h) files first to define the new naming
- Update implementation (.cpp) files
- Test compilation
- Fix any compilation errors
- Proceed to the next batch

### 4. Final Steps
- Final build test
- Run all unit tests
- Verify behavior

## Potential Issues
- Member variable shadowing in derived classes
- Name conflicts (field1_ vs _field1 from different classes)
- Variables passed by reference that assume the old naming
- Macros or preprocessor directives that rely on specific naming patterns

## Prioritized Classes to Update
1. Core/Object.h
2. Executor/Executor.h
3. Core/Registry.h
4. Language implementations