<img src="http://numenta.org/87b23beb8a4b7dea7d88099bfb28d182.svg" alt="NuPIC Logo" width=100/>

# htm.core

[![CI Build Status](https://github.com/htm-community/htm.core/workflows/build/badge.svg)](https://github.com/htm-community/htm.core/actions)

This is a Community Fork of the [nupic.core](https://github.com/numenta/nupic.core) C++ repository, with Python bindings. This implements the theory as described in [Numenta's BAMI](https://numenta.com/resources/biological-and-machine-intelligence/).

## Project Goals

- Actively developed C++ core library (Numenta's NuPIC repos are in maintenance mode only)
- Clean, lean, optimized, and modern codebase
- Stable and well tested code
- Open and easier involvement of new ideas across HTM community (it's fun to contribute, we make master run stable, but are more open to experiments and larger revamps of the code if it proves useful).
- Interfaces to other programming languages, currently C++ and Python

## Features

 * Implemented in C\+\+11 through C\+\+17
    + Static and shared lib files for use with C++ applications.
 * Interfaces to Python 3 and Python 2.7 (Only Python 3 under Windows)
 * Cross Platform Support for Windows, Linux, OSX and ARM64
 * Easy installation.  Many fewer dependencies than nupic.core, all are handled by CMake
 * Significant speed optimizations
 * Simplified codebase
    + Removed CapnProto serialization.  It was pervasive and complicated the
code considerably. It was replaced  with simple binary streaming serialization
in C++ library.
    + Removed sparse matrix libraries, use the optimized Connections class instead
 * New and Improved Algorithms
    + Revamped all algorithms APIs, making it easier for developers & researchers to use our codebase
    + Sparse Distributed Representation class, integration, and tools for working with them
 * API-compatibility with Numenta's code.
   An objective is to stay close to the [Nupic API Docs](http://nupic.docs.numenta.org/stable/api/index.html).
   This is a priority for the `NetworkAPI`.
   The algorithms APIs on the other hand have deviated from their original API (but their logic is the same as Numenta's).
   If you are porting your code to this codebase, please follow the [API Differences](API_DIFFERENCES.md) and consult the [API Changelog](API_CHANGELOG.md).
 * The 'NetworkAPI' as originally defined by the NuPIC library includes a set of build-in Regions. These   are described in [NetworkAPI docs](docs/NetworkAPI.md) 
 * REST interface for `NetworkAPI` with a REST server.

## Installation


### Prerequisites

For running C++ apps/examples/tests from binary release: none. 
If you want to use python, then obviously:

- [Python](https://python.org/downloads/)
    - Standard Python 3.7+ (Recommend using the latest)  [Tested with 3.8, 3.11.1, 3.12.4]
    - [Anaconda Python](https://www.anaconda.com/products/individual#Downloads) 3.7+
      + On windows you must run from within 'Anaconda Prompt' not 'Command Prompt'.
      + Anaconda Python is not tested in our CI.

  Be sure that your Python executable is in the Path environment variable.
  The Python that is in your default path is the one that will determine which
  version of Python the extension library will be built for.
  - Other implementations of Python may not work.
  - Only the standard python from python.org have been tested.
  - On Linux you will need both the Python install and the Python-dev install
        '$ sudo apt install python3.11'
        '$ sudo apt install python3.11-dev'
  - You will probably also want to setup a [https://docs.python.org/3/library/venv.html(venv environment).

- **C\+\+ compiler**: c\+\+11/17 compatible (ie. g++, clang\+\+).
    On Windows, tested with MSVC 2019, 2022
  - boost library (if not a C\+\+17 or greater compiler that supports filesystem.) 
    If the build needs boost, it will automatically download and install boost with the options it needs.
  - CMake 3.7+  (MSVC 2019 needs CMake 3.14+, MSVC 2022 needs CMake 3.21+).  
    Install the latest using [https://cmake.org/download/](https://cmake.org/download/)

Note: Windows MSVC 2019 and up runs as C\+\+17 by default so boost is not needed.  On linux use -std=c++17 compile option to avoid needing boost.

### Building from Source

An advantage of `HTM.core` is its well tested self-sustained dependency install, so you can install
HTM on almost any platform/system if installing from source. 

Fork or [download](https://github.com/htm-community/htm.core/archive/master.zip) the HTM-Community htm.core repository from [https://github.com/htm-community/htm.core](https://github.com/htm-community/htm.core). 

To fork the repo with `git`:
```
git clone https://github.com/htm-community/htm.core
```


#### Simple Python build (any platform)

1) Prerequisites: install the following python packages:
```
    python -m ensurepip --upgrade
    python -m pip install setuptools packaging
    pip install -r requirements.txt
```
    Note: requirements.txt limits numpy to version 1.23. You can use a newer
	      version (i.e. 2.x) if you use at least Python 3.9.
		  
2) At a command prompt, `cd` to the root directory of this repository.

3) Run: `python setup.py install --user --force`

   This will build and install a release version of htm.core.  The `--user` option prevents the system installed site-packages folder from being changed and avoids the need for admin privileges.  The `--force` option forces the package to be replaced if it already exists from a previous build. Alternatively you can type `pip uninstall htm.core` to remove a previous package before performing a build.
   
   * If you are using `virtualenv` you may not need the --user or --force options.
   * If you are using Anaconda Python you must run within the `Anaconda Prompt` on Windows. Do not use --user or --force options.

   * If you run into problems due to caching of arguments in CMake, delete the
   folder `<path-to-repo>/build` and try again.  This may be only an issue when you restart a build after a failure.

4) After that completes you are ready to import the library:
    ```python
    python.exe
    >>> import htm           # Python Library
    >>> import htm.bindings  # C++ Extensions
    >>> help( htm )          # Documentation
    ```
    
    You can run the unit tests with
    
    ```python
    python setup.py test
    ```


#### Simple C++ build 

After cloning/downloading the repository, do the following:
```
cd path-to-repository
mkdir -p build/scripts
cd build/scripts
cmake ../..
make -j8 install
```

| Build Artifact | File Location |
| :------------- | :------------ |
| Static Library         | `build/Release/lib/libhtm-core.a`    |
| Shared Library         | `build/Release/lib/libhtm-core.so`   |
| Header Files           | `build/Release/include/`             |
| Unit Tests             | `build/Release/bin/unit_tests`       |
| Hotgym Dataset Example | `build/Release/bin/benchmark_hotgym` |
| MNIST Dataset Example  | `build/Release/bin/mnist_sp`         |
| REST Server Example    | `build/Release/bin/rest_server`      |
| REST Client Example    | `build/Release/bin/rest_client`      |

 * A debug library can be created by adding `-DCMAKE_BUILD_TYPE=Debug` to the cmake command above.
   + The debug library will be put in `build/Debug` rather than `build/Release`.
     Use the cmake option `-DCMAKE_INSTALL_PREFIX=../Release` to change this.

 * The -j option can be used with the `make install` command to compile with multiple threads.

 * This will not build the Python interface. Use the Python build described above to build and install the python interface.

Here is an example of a **Release build** of your own C++ app that links to htm.core as a shared library.
```
#! /bin/sh
# Using GCC on linux ...
# First build htm.core from sources.
#      cd <path-to-repo>
#      mkdir -p build/scripts
#      cd build/scripts
#      cmake ../..
#      make -j4 install
#
# Now build myapp
# We use -std=c++17 to get <filesystem> so we can avoid using the boost library.
# The -I gives the path to the includes needed to use with the htm.core library.
# The -L gives the path to the shared htm.core library location at build time.
# The LD_LIBRARY_PATH envirment variable points to the htm.core library location at runtime.
g++ -o myapp -std=c++17 -I <path-to-repo>/build/Release/include myapp.cpp -L <path-to-repo>/build/Release/lib -lhtm_core -lpthread -ldl

# Run myapp 
export LD_LIBRARY_PATH=<path-to-repo>/build/Release/lib:$LD_LIBRARY_PATH
./myapp
```

Here is an example of a **Debug build** of your own C++ app that links to htm.core as a shared library.
```
#! /bin/sh
# Using GCC on linux ...
# First build htm.core as debug from sources.
#      cd <path-to-repo>
#      mkdir -p build/scripts
#      cd build/scripts
#      cmake ../.. -DCMAKE_BUILD_TYPE=Debug
#      make -j4 install
#
# Now build myapp
# The -g -Og tells the compiler to build debug mode with no optimize.
# We use -std=c++17 to get <filesystem> so we can avoid using the boost library.
# The -D_GLIBCXX_DEBUG setting tell compiler to compile std:: with debug
# The -I gives the path to the includes needed to use with the htm.core library.
# The -L gives the path to the shared htm.core library location at build time.
# The LD_LIBRARY_PATH envirment variable points to the htm.core library location at runtime.
g++ -g -Og -o myapp -std=c++17 -D_GLIBCXX_DEBUG -I <path-to-repo>/build/Debug/include myapp.cpp -L <path-to-repo>/build/Debug/lib -lhtm_core -lpthread -ldl

# Run myapp in the debugger
export LD_LIBRARY_PATH=<path-to-repo>/build/Debug/lib:$LD_LIBRARY_PATH
gdb ./myapp
```

### Docker Builds

#### Build for Docker amd64 (x86_64)

Our [Dockerfile](./Dockerfile) allows easy (cross) compilation from/to many HW platforms. This docker file does the full build, test & package build. 
It takes quite a while to complete. 

If you are on `amd64` (`x86_64`) and would like to build a Docker image:

```sh
docker build --build-arg arch=amd64 .
```

#### Docker build for ARM64

If you are on `ARM64` and would like to build a Docker image, run the command
below. The CI automated ARM64 build (detailed below) uses this
specifically.

```sh
docker build --build-arg arch=arm64 .
```
Note: 
* If you're directly on ARM64/aarch64 (running on real HW) you don't need the docker image, and can use the standard binary/source installation procedure. 

#### Docker build for ARM64/aarch64 on AMD64/x86_64 HW

A bit tricky part is providing cross-compilation builds if you need to build for a different platform (aarch64) then your system is running (x86_64). 
A typical case is CI where all the standard(free) solutions offer only x86_64 systems, but we want to build for ARM. 

See our [ARM release workflow](./.github/workflows/release.yml). 

When running locally run:
```sh
docker run --privileged --rm multiarch/qemu-user-static:register
docker build -t htm-arm64-docker --build-arg arch=arm64 -f Dockerfile-pypi .
docker run htm-arm64-docker uname -a
docker run htm-arm64-docker python setup.py test
```
Note: 
* the 1st line allows you to emulate another platform on your HW.
* 2nd line builds the docker image. The [Dockerfile](./Dockerfile) is a lightweight Alpine_arm64 image, which does full build,test&package build. It can take quite a long time. 
  The [Dockerfile-pypi](./Dockerfile-pypi) "just" switches you to ARM64/aarch64 env, and then you can build & test yourself.


### Automated Builds, CI

We use Github `Actions` to build and run multiplatform (OSX, Windows, Linux, ARM64) tests and releases. 
* the [pr.yml](/.github/workflows/pr.yml) runs on each pull-request (PR), builds for Linux(Ubuntu 20.04), Windows(2019), OSX(10.15) and checkes that all tests pass OK. This is mandatory for a new PR
to be accepted. 
* [release.yml](/.github/workflows/release.yml) is created manually by the maintainers in the [release process](./RELEASE.md) and creates 
  - binary GitHub releases
  - PyPI wheels for `htm.core`
  - uploads artifacts
* [arm.yml](/.github/workflows/arm.yml) is an ARM64 build (that takes a long time) and thus is ran only daily.



[![CI Build Status](https://github.com/htm-community/htm.core/workflows/pr/badge.svg)](https://github.com/htm-community/htm.core/actions)

#### Linux/OSX/Windows auto build on PR @ Github Actions

 * [![CI Build Status](https://github.com/htm-community/htm.core/workflows/pr/badge.svg)](https://github.com/htm-community/htm.core/actions?workflow=pr)
 * [Config](./.github/workflows/pr.yml)


#### ARM64 auto build @ Github Actions

This uses Docker and QEMU to achieve an ARM64 build on Actions' x86_64/amd64 hardware.

 * [![CI Build Status](https://github.com/htm-community/htm.core/workflows/arm64/badge.svg)](https://github.com/htm-community/htm.core/actions?workflow=arm64)
 * [Config](./.github/workflows/arm.yml)



### Documentation
For Doxygen see [docs README](docs/README.md).
For NetworkAPI see [NetworkAPI docs](docs/NetworkAPI.md).



## Workflow

### Using IDE  (Netbeans, XCode, Eclipse, KDevelop, etc)

Generate IDE solution & build.

 * Choose the IDE that interest you (remember that IDE choice is limited to your OS).
 * Open CMake executable in the IDE.
 * Specify the source folder (`$HTM_CORE`) which is the location of the root CMakeList.exe.
 * Specify the build system folder (`$HTM_CORE/build/scripts`), i.e. where IDE solution will be created.
 * Click `Generate`.

#### For MS Visual Studio 2017, 2019 or 2022 as the IDE

After downloading the repository, do the following:
 * NOTE: Visual Studio 2019 requires CMake version 3.14 or higher.
 *       Visual Studio 2022 requires CMake version 3.21 or higher.
 * CD to the top of repository.
 * Double click on `startupMSVC.bat`
    - This will setup the build, create the solution file (build/scripts/htm.cpp.sln), and start MS Visual Studio.
 * Select `Release` or `Debug` as the Solution Configuration. Solution Platform must remain at x64.
 * Build everything.  This will build the C++ library.
 * In the solution explorer window, right Click on 'unit_tests' and select `Set as StartUp Project` so debugger will run unit tests.
 * If you also want the Python extension library; then delete the `build` folder and then in a command prompt, cd to root of repository and run `python setup.py install --user --force`.

#### For Visual Studio Code (VSCode) as the IDE
[Visual Studio Code](https://code.visualstudio.com/) can be used on any of our three platforms (Windows, Linux, OSx). 
You will need the C/C++ Tools extension by Microsoft and CMake Tools by vector-of-bool.

Startup Visual Studio Code and open the folder containing your htm.core repository which will set the workspace. Let it scan for a kit.  Clear all of the notifications (lower right) so it can let you do the scan.

Then set your project level settings by initializing  <repository>/.vscode/settings.json to the following as a starting point.

For Windows 10:
```
  .vscode\settings.json          
    {
    "cmake.buildDirectory": "${workspaceRoot}/build/scripts",
    "cmake.generator": "Visual Studio 16 2019",
    "cmake.platform": "x64",
    }
```
To use Visual Studio 2017 as the tool chain, change generator to "Visual Studio 15 2017" and set the platform to "win32".  Note that the ninja generator, the default, did not work very well on Windows.

For Ubuntu and OSx:
```
   .vscode/settings.json
    {
    "cmake.buildDirectory": "${workspaceRoot}/build/scripts",
    "cmake.generator": "gcc",
    }
```


#### For Eclipse as the IDE
 * File - new C/C++Project - Empty or Existing CMake Project
 * Location: (`$HTM_CORE`) - Finish
 * Project properties - C/C++ Build - build command set "make -C build/scripts VERBOSE=1 install -j [number of your's CPU cores]"
 * There can be issue with indexer and boost library, which can cause OS memory to overflow -> add exclude filter to
   your project properties - Resource Filters - Exclude all folders that matches boost, recursively
 * (Eclipse IDE for C/C++ Developers, 2019-03 on Ubuntu 18.04)

For all new work, tab settings are at 2 characters, replace tabs with spaces.
The clang-format is LLVM style.


### Debugging 

Creating a debug build of the `htm.core` library and unit tests is the same as building any C++ 
application in Debug mode in any IDE as long as you do not include the python bindings. i.e. do 
not include `-DBINDING_BUILD=Python3` in the CMake command.
```
(on Linux)
   rm -r build
   mkdir -p build/scripts
   cd build/scripts
   CMake -DCMAKE_BUILD_TYPE=Debug ../..
```

However, if you need to debug the python bindings using an IDE debugger it becomes a little more difficult. 
The problem is that it requires a debug version of the python library, `python37_d.lib`.  It is possible to
obtain one and link with it, but a way to better isolate the python extension is to build a special `main( )`
as [explained in debugging Python](https://pythonextensionpatterns.readthedocs.io/en/latest/debugging/debug_in_ide.html).

Be aware that the CMake maintains a cache of build-time arguments and it will ignore some arguments passed
to CMake if is already in the cache.  So, between runs you need to clear the cache or even better,
entirely remove the `build/` folder (ie. `git clean -xdf`).

### Python development mode

When you run `python setup.py install --user --force` it will copy python scripts into `build/Release/distr/src` and deploy as package into user site-packages (on linux in `/home/.local/`).
To avoid deploying there use "development mode":
`python setup.py develop --user --force`
This will create link file in site-packages pointing to the distr folder. You can modify distr scripts and your changes will be reflected immediately.
Note: Unfortunately calling this command again will not overwrite distr scripts, so you need to delete distr folder first.

To remove the link file call:

`python setup.py develop --user --uninstall`

Note: you can always check from where you are importing sources, by typing into python console e.g.:
```
import htm.bindings.sdr
print(htm.bindings.sdr.__file__)
```
Note2: It is obvious, but anyway - do not use `--user` option while using python environment managers(Anaconda..)

### Dependency management

The installation scripts will automatically download and build the dependencies it needs.

 * [Boost](https://www.boost.org/)   (Not needed by C++17 compilers that support the filesystem module)
 * [LibYaml](https://pyyaml.org/wiki/LibYAML)
 * [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
 * [PyBind11](https://github.com/pybind/pybind11)
 * [gtest](https://github.com/google/googletest)
 * [cereal](https://uscilab.github.io/cereal/)
 * [mnist test data](https://github.com/wichtounet/mnist)
 * [sqlite3](https://www.sqlite.org/2020/sqlite-autoconf-3320300.tar.gz)
 * [digestpp](https://github.com/kerukuro/digestpp) (for SimHash encoders)
 * and [python requirements.txt](./requirements.txt)

Once these third party components have been downloaded and built they will not be
re-visited again on subsequent builds.  So to refresh the third party components
or rebuild them, delete the folder `build/ThirdParty` and then re-build.

If you are installing on an air-gap computer (one without Internet) then you can
manually download the dependencies.  On another computer, download the
distribution packages as listed and rename them as indicated. Copy these to
`${REPOSITORY_DIR}/build/ThirdParty/share` on the target machine.

| Name to give it        | Where to obtain it |
| :--------------------- | :----------------- |
| libyaml.zip            | https://github.com/yaml/libyaml/archive/refs/tags/0.2.5.tar.gz |
| boost.tar.gz  (*note3) | https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz | 
| googletest.tar.gz      | https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz |
| eigen.tar.bz2          | https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz |
| mnist.zip     (*note4) | https://github.com/wichtounet/mnist/archive/3b65c35ede53b687376c4302eeb44fdf76e0129b.zip |
| pybind11.tar.gz        | https://github.com/pybind/pybind11/archive/refs/tags/v2.10.1.tar.gz |
| cereal.tar.gz          | https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.tar.gz |
| sqlite3.tar.gz         | https://www.sqlite.org/2022/sqlite-autoconf-3380200.tar.gz |
| digestpp.zip           | https://github.com/kerukuro/digestpp/archive/34ff2eeae397ed744d972d86b5a20f603b029fbd.zip |
| cpp-httplib.zip(*note4)| https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.3.zip |

 * note3: Boost is not required for any compiler that supports C++17 with `std::filesystem` (MSVC2017, gcc-8, clang-9).
 * note4: Used for examples. Not required to run but the build expects it.


## Testing

We support test-driven development with reproducible builds. 
You should run tests locally, and tests are also run as a part of the CI. 

### C++ & Python Unit Tests:

There are two sets (somewhat duplicit) tests for c++ and python.

 * C++ Unit tests -- to run: `./build/Release/bin/unit_tests`
 * Python Unit tests -- to run: `python setup.py test` (runs also the C++ tests above)
   - `py/tests/`
   - `bindings/py/tests/`


## Examples

### Python Examples

There are a number of python examples, which are runnable from the command line.
They are located in the module `htm.examples`.

Example Command Line Invocation: `$ python -m htm.examples.sp.hello_sp`

Look in: 
- `py/htm/examples/`
- `py/htm/advanced/examples/`

### Hot Gym

This is a simple example application that calls the `SpatialPooler` and
`TemporalMemory` algorithms directly.  This attempts to predict the electrical
power consumption for a gymnasium over the course of several months.

To run python version:
```
python -m htm.examples.hotgym
```

To run C++ version: (assuming current directory is root of the repository)
```sh
./build/Release/bin/benchmark_hotgym
```

There is also a dynamically linked version of Hot Gym (not available on MSVC). 
You will need specify the location of the shared library with `LD_LIBRARY_PATH`.

To run: (assuming current directory is root of the repository)
```sh
LD_LIBRARY_PATH=build/Release/lib ./build/Release/bin/dynamic_hotgym
```

### MNIST benchmark

The task is to recognize images of hand written numbers 0-9.
This is often used as a benchmark.  This should score at least 95%.

To run: (assuming current directory is top of repository)
```
  ./build/Release/bin/mnist_sp
```

In Python: 
```
python py/htm/examples/mnist.py
```

### REST example

The REST interface for NetworkAPI provides a way to access the underlining htm.core library
using a REST client.  The examples provide both a full REST web server that can process the web
requests that allow the user to create a Network object resource and perform htm operations on it.
Message layout details can be found in [NetworkAPI REST docs](docs/NetworkAPI_REST.md).
To run:
```
   ./build/Release/bin/server [port [network_interface]]
```

A REST client, implemented in C++ is also provided as an example of how to use the REST web server.
To run:  first start the server.
```
   ./build/Release/bin/client [host [port]]
```
The default host is 127.0.0.1 (the local host) and the port is 8050.


## License

The htm.core library is distributed under GNU Affero Public License version 3 (AGPLv3).  The full text of the license can be found at http://www.gnu.org/licenses.

Libraries that are incorporated into htm.core have the following licenses:

| Library | Source Location | License |
| :------ | :-------------- | :------ |
| libyaml | https://github.com/yaml/libyaml | https://github.com/yaml/libyaml/blob/master/LICENSE |
| boost (*note3)  | https://www.boost.org/      | https://www.boost.org/LICENSE_1_0.txt |
| eigen   | http://eigen.tuxfamily.org/ | https://www.mozilla.org/en-US/MPL/2.0/ |
| pybind11 | https://github.com/pybind/pybind11 | https://github.com/pybind/pybind11/blob/master/LICENSE |
| cereal | https://uscilab.github.io/cereal/ | https://opensource.org/licenses/BSD-3-Clause |
| digestpp | https://github.com/kerukuro/digestpp | released into public domain |
| cpp-httplib | https://github.com/yhirose/cpp-httplib | https://github.com/yhirose/cpp-httplib/blob/master/LICENSE |

 * note3: Boost is not used if built with any compiler that supports C++17 with `std::filesystem` (MSVC2017, gcc-8, clang-9).
 
 
## Cite us

We're happy that you can use the community work in this repository or even join the development! 
Please give us attribution by linking to us as [htm.core](https://github.com/htm-community/htm.core/) at https://github.com/htm-community/htm.core/ , 
and for papers we suggest to use the following BibTex citation: 

```
@misc{htmcore2019,
	abstract = "Implementation of cortical algorithms based on HTM theory in C++ \& Python. Research \& development library.",
	author = "M. Otahal and D. Keeney and D. McDougall and others",
	commit = bf6a2b2b0e04a1d439bb0492ea115b6bc254ce18,
	howpublished = "\url{https://github.com/htm-community/htm.core/}",
	journal = "Github repository",
	keywords = "HTM; Hierarchical Temporal Memory; NuPIC; Numenta; cortical algorithm; sparse distributed representation; anomaly; prediction; bioinspired; neuromorphic",
	publisher = "Github",
	series = "{Community edition}",
	title = "{HTM.core implementation of Hierarchical Temporal Memory}",
	year = "2019"
}
```
> Note: you can update the commit to reflect the latest version you have been working with to help 
making the research reproducible. 


## Helps
[Numenta's BAMI](https://numenta.com/resources/biological-and-machine-intelligence/) The formal theory behind it all. Also consider [Numenta's Papters](https://numenta.com/neuroscience-research/research-publications/papers/).

[HTM School](https://numenta.org/htm-school/)  is a set of videos that explains the concepts.

Indy's Blog
* [Hierarchical Temporal Memory – part 1 – getting started](https://3rdman.de/2020/02/hierarchical-temporal-memory-part-1-getting-started/)
* [Hierarchical Temporal Memory – part 2](https://3rdman.de/2020/04/hierarchical-temporal-memory-part-2/)

For questions regarding the theory can be posted to the [HTM Forum](https://discourse.numenta.org/categories). 

Questions and bug reports regarding the library code can be posted in [htm.core Issues blog](https://github.com/htm-community/htm.core/issues).

## Related community work

Community projects for working with HTM. 

### Visualization
#### HTMPandaVis
This project aspires to create tool that helps **visualize HTM systems in 3D** by using opensource framework for 3D rendering https://www.panda3d.org/

NetworkAPI has region called "DatabaseRegion". This region can be used for generating SQLite file and later on read by PandaVis - DashVis feature,
to show interactive plots in web browser on localhost. See [napi_hello_database](https://github.com/htm-community/htm.core/tree/master/src/examples/napi_hello) for basic usage.

For more info, visit [repository of the project](https://github.com/htm-community/HTMpandaVis)
![pandaVis1](docs/images/pandaVis1.png)


