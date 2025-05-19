# Installing

KAI is cross-platform across maxOS, Linux and Windows system with many internal and external components. It also has 3rd party dependencies. Don't worry too much: if the 3rd party systems can't be found to make things like a [Windowed gui](Source/App/Window) you will still get the [TestSuite](/Test) and colored text-base [Console](Source/App/Console).

## Downloading and updating

```bash
> git clone git@github.com:cschladetsch/KAI.git
> git submodule init
> git submodule update
```

Currently the list of external `git` repos are:

* *RakNet*. Cross-platform networking.
* *rang*. Cross-platform Colored console output.
* *imgui*. Cross-platform Immediate-mode windowing system.

## Boost

Install Boost 1.72 or higher.

For Windows:

```bash
> .\bootstrap.bat
> .\b2 install debug --date-time --build=complete --with-chrono --with-filesystem --with-system --with-program_options
```

The same is true for Linux or macOS, just use `./` instead of `.\`

## Building the Project

Please see [BUILD.md](./BUILD.md) for detailed instructions on building KAI.

### Quick Start (Ubuntu/macOS)

```bash
# Using helper script (recommended)
./b    # Build with Clang++ (default)
# or
./b --gcc  # Build with GCC

# Alternative: Use the Makefile
make       # Build with Clang++ (default)
# or
make gcc   # Build with GCC
```

### Legacy Method (Ubuntu/macOS)

```bash
> mkdir -p build && cd build
> cmake ..                        # Uses Clang++ by default
# or
> cmake .. -DBUILD_GCC=ON         # Use GCC
> cmake --build .
```

## Windows

You will need [cmake](https://cmake.org/download/) and [boost](https://sourceforge.net/projects/boost/files/boost-binaries/) installed with at least `chrono`, `system` and `filesystem` libraries. 

### Quick Start (Windows)

```batch
:: Clean any existing CMake artifacts and set up build directory
.\clean_build.bat

:: Build the project
.\build.bat
```

### Legacy Method (Windows)

```batch
> mkdir build
> cd build
> cmake ..
> cmake --build .
:: Or open the solution file
> start *.sln
```

From there, build the "ALL_BUILD" project, or select a specific Test or App to test.

I suggest you include the [GoogleTest integration](www.googe.com/?q=findit) extension for Visual Studio under windows.

## Folders

The resulting interesting folders:

* __KAI/Bin__. Executables and test suites.
* __KAI/Include__. Root C++ include folder for KAI systems.
* __KAI/Lib__. C++ static and shared libraries

