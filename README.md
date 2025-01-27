<img src="http://numenta.org/87b23beb8a4b7dea7d88099bfb28d182.svg" alt="NuPIC Logo" width=100/>

# htm.core

[![CI Build Status](https://github.com/htm-community/htm.core/workflows/build/badge.svg)](https://github.com/htm-community/htm.core/actions)

This is a Community Fork of the [nupic.core](https://github.com/numenta/nupic.core) C++ repository, with Python bindings. This implements the theory as described in [Numenta's BAMI](https://numenta.com/resources/biological-and-machine-intelligence/).

## Project Goals

- Actively developed C++ core library (Numenta's NuPIC repos are in archive mode)
- Clean, lean, optimized, and modern codebase
- Stable and well tested code
- Open and easier involvement of new ideas across HTM community (it's fun to contribute, 
we make the master run stable, but we are open to experiments and revamps of the code 
if it proves useful).
- Interfaces to C++ and Python

## Features

 * Implemented in C\+\+17 and above.
    both Static lib and shared lib .so for use with C++ applications.
 * C++ Extensions for Python3 using pybind11 which exposes all htm.core library functions.
 * Cross Platform Support for Windows, Linux, OSX.
 * Easy installation.  Few dependencies and most are obtained automatically by CMake during the build.
 * Sparse Distributed Representation class, integration, and tools for working with them.
 * API-compatibility with Numenta's original Python 2.0 code.
   An objective is to stay close to the [Nupic API Docs](http://nupic.docs.numenta.org/stable/api/index.html).
   This is a priority for the `NetworkAPI`.
   The algorithms APIs on the other hand have deviated from their original API 
   (but their basic logic is the same as Numenta's).
   If you are porting your code to this codebase, please follow the 
   [API Differences](API_DIFFERENCES.md) and consult the [API Changelog](API_CHANGELOG.md).
 * The 'NetworkAPI', as originally defined by the NuPIC library, includes a set of build-in Regions. 
 These are described in [NetworkAPI docs](docs/NetworkAPI.md) 
 * REST interface for `NetworkAPI` with a REST server.

## Installation


### Prerequisites

For running C++ apps/examples/tests from binary release: none. 
If you want to use python, then obviously:

- [Python](https://python.org/downloads/)
    - Standard Python 3.9+ (Recommend using the latest)  [Tested with 3.11.1, 3.12.4, 3.13]
	  + Must be running in a virtual environment such as venv.
    - [Anaconda Python](https://www.anaconda.com/products/individual#Downloads) 3.9+
      + On windows you must run from within 'Anaconda Prompt' not 'Command Prompt'.
      + Anaconda Python is not tested in our CI.

  Only the standard python from python.org have been tested.
  On Linux you will need both the Python install and the Python-dev install
        '$ sudo apt install python3.13'
        '$ sudo apt install python3.13-dev'
  - You will also want to setup a [https://docs.python.org/3/library/venv.html(venv environment).

- **C\+\+ compiler**: c\+\+17 compatible (ie. g++, clang\+\+).
    On Windows, tested with MSVC 2019, 2022
  - CMake 3.21+.  
    Install the latest CMake using [https://cmake.org/download/](https://cmake.org/download/)

Note: Windows MSVC 2019 and up runs as C\+\+17 by default.  On linux use -std=c++17 if not default.

### Building from Source

An advantage of `HTM.core` is its well tested self-sustained dependency install, so you can install
HTM on almost any platform/system if installing from source. 

Fork or [download](https://github.com/htm-community/htm.core/archive/master.zip) 
the HTM-Community htm.core repository from [https://github.com/htm-community/htm.core](https://github.com/htm-community/htm.core). 

To fork the repo with `git`:
```
git clone https://github.com/htm-community/htm.core
```


#### Simple Source build (any platform)

1) Prerequisites: 
    Must create a Python virtual environment.
	Create a virtual environment: 	`python3 -m venv .venv`

	Activate the virtual environment: `.venv/bin/activate` (or .venv\Scripts\activate on Windows)

		  

2) Build and install: 
    ```
	cd <project directory>
	python htm_install.py
	```

   This will obtain all prerequisites build a release version of htm.core,
   then it will install the htm package into the Python environment 
   that was used for the build.  The C++ library (htm_core.lib), include files and executables
   can be found in build/Release.
   

    The C++ portion of the build leaves cmake cache and build artifacts in 
	the folder build. If the build needs to be restarted, delete the build folder.
	
    The Python portion of the build is performed in an isolated environment in a temp folder.
	When the build completes, the build artifacts and cache are deleted.

    Note that if you want to build in a virgin uv environment you will need to install at least 
    `pip` in this environment and activate the environment. So the steps would be:
    ```
	cd <project directory>
    uv venv --python <python version, e.g. 3.12>
    uv pip install pip
    source .venv/bin/activate
	python htm_install.py
    ```



3) After the build completes you are ready to import the library:
    ```shell
    python.exe
    >>> import htm           # Python Library
    >>> import htm.bindings  # C++ Extensions
    >>> help( htm )          # Documentation
    ```
    
    You can run the unit tests with
    
    ```shell
    python htm_test.py
    ```


     The following C++ build artifacts can be found in build/Release

| Build Artifact | File Location |
| :------------- | :------------ |
| Static Library         | `build/Release/lib/libhtm-core.a`    |
|                        | `build/Release/lib/htm_core.lib      |
| Shared Library         | `build/Release/lib/libhtm-core.so`   |
|                        |  (Linux only)                        |
| Header Files           | `build/Release/include/`             |
| Unit Tests             | `build/Release/bin/unit_tests`       |
| Hotgym Dataset Example | `build/Release/bin/benchmark_hotgym` |
| MNIST Dataset Example  | `build/Release/bin/mnist_sp`         |
| REST Server Example    | `build/Release/bin/rest_server`      |
| REST Client Example    | `build/Release/bin/rest_client`      |


	  
# If you want to build a C++ app that calls the C++ htm_core library.
# The -I gives the path to the includes needed to use with the htm.core library.
# The -L gives the path to the shared htm.core library location at build time.
# The LD_LIBRARY_PATH envirment variable points to the htm.core library location at runtime.
`g++ -o myapp -std=c++17 -I <path-to-repo>/build/Release/include myapp.cpp -L <path-to-repo>/build/Release/lib -lhtm_core -lpthread -ldl`

# Run myapp 
```
export LD_LIBRARY_PATH=<path-to-repo>/build/Release/lib:$LD_LIBRARY_PATH
./myapp
```

### AirGap computer builds (target machine with no internet)
Refer to instructions for [AirGap installs](AirGapBuild.md).

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

When running locally, run:
```sh
docker run --privileged --rm multiarch/qemu-user-static:register
docker build -t htm-arm64-docker --build-arg arch=arm64 -f Dockerfile-pypi .
docker run htm-arm64-docker uname -a
docker run htm-arm64-docker python htm_test.py
```
Note: 
* the 1st line allows you to emulate another platform on your HW.
* 2nd line builds the docker image. The [Dockerfile](./Dockerfile) is a lightweight Alpine_arm64 image, which does full build,test&package build. It can take quite a long time. 
  The [Dockerfile-pypi](./Dockerfile-pypi) "just" switches you to ARM64/aarch64 env, and then you can build & test yourself.



### Documentation
For Doxygen see [docs README](docs/README.md).
For NetworkAPI see [NetworkAPI docs](docs/NetworkAPI.md).

The library entry point documentation can be obtained using the help call.
    ```shell
    python
    >>> import htm           # Python Library
    >>> import htm.bindings  # C++ Extensions
    >>> help( htm )          # Documentation
    ```




### Dependency management

The installation script (python htm_install.py) will automatically download and build the 
dependencies it needs.

 * [LibYaml](https://pyyaml.org/wiki/LibYAML)
 * [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
 * [gtest](https://github.com/google/googletest)
 * [cereal](https://uscilab.github.io/cereal/)
 * [mnist](https://github.com/wichtounet/mnist)
 * [sqlite3](https://www.sqlite.org/2020/sqlite-autoconf-3320300.tar.gz)
 * [digestpp](https://github.com/kerukuro/digestpp) (for SimHash encoders)
 * It also downloads and installs all python packages needed for the build and execution.


If you are installing on an air-gap computer (one without Internet) then you can
manually download the dependencies.  On another computer, download the
distribution packages as listed. Copy these to
`${REPOSITORY_DIR}/build/ThirdParty/` on the target machine.
Unzip the archive and rename the folder as in the left column.

| Name to give folder    | Where to obtain it |
| :--------------------- | :----------------- |
| libyaml            | https://github.com/yaml/libyaml/archive/refs/tags/0.2.5.tar.gz |
| gtest              | https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz |
| eigen              | https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz |
| mnist  (*note1)    | https://github.com/wichtounet/mnist/archive/3b65c35ede53b687376c4302eeb44fdf76e0129b.zip |
| cereal             | https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.tar.gz |
| sqlite3            | https://www.sqlite.org/2024/sqlite-amalgamation-3470000.zip |
| digestpp           | https://github.com/kerukuro/digestpp/archive/34ff2eeae397ed744d972d86b5a20f603b029fbd.zip |
| cpp-httplib(*note1)| https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.1.zip |

 * note1: Used for examples. Not required to run but the build expects it.


## Testing

We support test-driven development with reproducible builds. 
You should run tests locally. 

### C++ & Python Unit Tests:

There are two sets (somewhat duplicit) tests for c++ and python.

 * C++ Unit tests -- to run: `./build/Release/bin/unit_tests`
 * Python Unit tests -- to run: `python -m pytest` 
Run both by running `python htm_test.py`



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
| eigen   | http://eigen.tuxfamily.org/ | https://www.mozilla.org/en-US/MPL/2.0/ |
| pybind11 | https://github.com/pybind/pybind11 | https://github.com/pybind/pybind11/blob/master/LICENSE |
| cereal | https://uscilab.github.io/cereal/ | https://opensource.org/licenses/BSD-3-Clause |
| digestpp | https://github.com/kerukuro/digestpp | released into public domain |
| cpp-httplib | https://github.com/yhirose/cpp-httplib | https://github.com/yhirose/cpp-httplib/blob/master/LICENSE |

 
 
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

NetworkAPI has a region called "DatabaseRegion". This region can be used for generating SQLite file and later on read by PandaVis - DashVis feature,
to show interactive plots in web browser on localhost. See [napi_hello_database](https://github.com/htm-community/htm.core/tree/master/src/examples/napi_hello) for basic usage.

For more info, visit [repository of the project](https://github.com/htm-community/HTMpandaVis)
![pandaVis1](docs/images/pandaVis1.png)


