## Building on Air-Gapped Systems

Building on an air-gapped system (a system without direct internet access) 
requires a few extra steps to ensure that all necessary dependencies are available.
In addition to your Air-Gapped computer, you will need access to a computer that has 
access to the internet.  

An alternative to doing all of the steps below, is to do the entire build on the Online Machine
Then copy the wheel to the target machine (copy entire `htm.core` project directory). Then on the target machine, 
run `python htm_install.py` to install. The Online Machine must be the same platform as the Air-Gap Machine 
and use the same python version.

But if you do not want to build on the Online Machine, here are the steps to obtain the dependencies
so that the build can be performed on the Air Gap machine.

**Prerequisites for OnLine Machine:**
*   **This Online Machine should be one with a platform that is simular to the Air-Gapped 
machine to avoid complications in transferring text with different line endings. If you want to 
use a different platform, be sure you handle line endings in text files when tranfering. 
See notes below.
*   **Python:** Python 3.9 or higher, along with `pip`, must be installed. 
Use `$ python --version` to confirm.

**Prerequisites for Air-Gapped Machine:**
*   **Python:** Python 3.9 or higher, along with `pip`, must be installed. 
Use `$ python --version` to confirm. If not, separatly download and install the 
latest official python from [Python.org/downloads](Python.org/downloads).

**Steps:**

1.  **(Online Machine) Obtain the Project Source Code:**

    *   **Option A: Clone the repository:**
        If you have Git installed, clone the repository using:

        ```console
        $ git clone https://github.com/htm-community/htm.core.git
        ```

        This will create a directory named `htm.core` containing the project's source code.
        **Important:** If your online machine is a different platform than your air-gapped machine, 
		configure Git to handle line endings appropriately, as described in the "Important Notes" 
		section below.
		
    *   **Option B: Download a source archive:**
        If you don't have Git, or prefer a zip file, most Git repositories provide downloadable archives:
            *   Go to the repository's main page on GitHub: 
			     [https://github.com/htm-community/htm.core](https://github.com/htm-community/htm.core)
            *   Click the "Code" button.
            *   Select "Download ZIP".
            *   Extract the downloaded ZIP file to create the project directory.
			
	Once you have the project directory populated, cd to that folder. The rest of the steps
	assume your current directory is your project directory.

2.  **(Online Machine) Download and Prepare Dependencies:**

    *   Create a file `build/dependencies/requirements.txt` with the following content:

        ```
        scikit-build-core>=0.10.7
        cmake>=3.27
        pybind11>=2.6.0
        build
		numpy>=1.26.4
		<2.0 hexy>=1.4.4 
		prettytable>=3.5.0
		pytest>=6.0 
		mock>=3.3
        ```

    *   Download the necessary Python build-time dependencies into the `build/dependencies` folder:
        *   Open a command line window and run:

            ```shell
            $ pip download -r build/dependencies/requirements.txt --dest build/dependencies
            ```

    *   Download the C++ dependency archives listed below into the `htm.core` directory:
        *   **cereal:** [https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.tar.gz](https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.tar.gz)
        *   **eigen:** [https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz](https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz)
        *   **cpp-httplib:** [https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.1.zip](https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.1.zip)
        *   **digestpp:** [https://github.com/kerukuro/digestpp/archive/34ff2eeae397ed744d972d86b5a20f603b029fbd.zip](https://github.com/kerukuro/digestpp/archive/34ff2eeae397ed744d972d86b5a20f603b029fbd.zip)
        *   **gtest:** [https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz](https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz)
        *   **libyaml:** [https://github.com/yaml/libyaml/archive/refs/tags/0.2.5.tar.gz](https://github.com/yaml/libyaml/archive/refs/tags/0.2.5.tar.gz)
        *   **mnist:** [https://github.com/wichtounet/mnist/archive/3b65c35ede53b687376c4302eeb44fdf76e0129b.zip](https://github.com/wichtounet/mnist/archive/3b65c35ede53b687376c4302eeb44fdf76e0129b.zip)
        *   **sqlite3:** [https://www.sqlite.org/2024/sqlite-amalgamation-3470000.zip](https://www.sqlite.org/2024/sqlite-amalgamation-3470000.zip)
    *   Extract each of the downloaded C++ dependency archives directly into their corresponding `build/Thirdparty/<dependency_name>` folder within the project. For example, extract `cereal-1.3.2.tar.gz` into `htm.core/build/Thirdparty/cereal/`, `eigen-3.4.0.tar.gz` into `htm.core/build/Thirdparty/eigen/`, and so on. When finished, you should have a directory structure that looks like:

	The build folder should looks something like this:
        ```
        htm.core/
        +-- build/
            +-- Thirdparty/
            ¦   +-- cereal/
            ¦   ¦   +-- ...
            ¦   +-- eigen/
            ¦   ¦   +-- ...
            ¦   +-- cpp-httplib/
            ¦   ¦   +-- ...
            ¦   +-- digestpp
            ¦   ¦   +-- ...
            ¦   +-- gtest
            ¦   ¦   +-- ...
            ¦   +-- libyaml
            ¦   ¦   +-- ...
            ¦   +-- mnist
            ¦   ¦   +-- ...
            ¦   +-- sqlite3/
            ¦       +-- ...
            +-- dependencies/
                +-- requirements.txt
                +-- scikit-build-core-*.whl
                +-- cmake-*.whl
                +-- pybind11-*.whl
                +-- build-*.whl
                +-- numpy-*.whl
                +-- hexy-*.whl
                +-- prettytable-*.whl
                +-- pytest-*.whl    
                +-- mock-*.whl      
        ```

3.  **Transfer Project to Air-Gapped Machine:**
    Transfer the entire `htm.core` project directory to the air-gapped machine.
	You can use any means available but most likely it will be a Thumb drive.
	
	If the Online Machine is a different platform type, be sure to be careful about 
	the line endings on text files. See notes below.

4.  **(Air-Gapped Machine) Create a Virtual Environment:**
    *   Open a command window on the Air-Gapped machine.
    *   Navigate to the project folder (the transferred `htm.core` directory).
    *   Create a virtual environment using the built-in `venv` module:

        ```console
        $ python3 -m venv .venv
        ```

    *   Activate the virtual environment:
        *   **Linux/macOS:**

            ```shell
            $ source .venv/bin/activate
            ```

        *   **Windows (Command Prompt):**

            ```console
            $ .venv\Scripts\activate.bat
            ```

        *   **Windows (PowerShell):**

            ```console
            $ .venv\Scripts\Activate.ps1
            ```


5.  **(Air-Gapped Machine) Build the Project:**

    *   Navigate to the project directory (`htm.core`)
    *   Run the build command:

        ```console
        $ python htm_install.py
        ```

    *   The build system will automatically use the local overrides for the C++ dependencies 
	because they are present in the expected locations.
    *   This will take a while. but when finished the the htm package will be installed in Python.
	*   The C++ build artifacts will be found in build/Release
	

##**Important Notes:**

*   **Python Virtual Environment:** It is highly recommended to create and activate a virtual environment 
before installing dependencies or building the project. This ensures a clean and isolated build 
environment. The `venv` module is part of the Python standard library (Python 3.3+), so you don't 
need to download or install it separately.

*   **Line Endings:**
    *   If your online machine and air-gapped machine use different operating systems, be mindful 
	of line ending differences in text files (especially `requirements.txt`, `pyproject.toml`, 
	all `CMakeLists.txt` files, and the project's Python and C++ source files source code).
    *   If using Git, configure it to handle line endings automatically by adding a `.gitattributes` file to the root of your repository with the following content:

        ```
        * text=auto eol=lf
        *.{cpp,h,hpp,c,cc,cxx,cmake,in,py,md} text eol=lf
        *.bat text eol=crlf
        ```
    *   You may also need to configure your text editor or IDE to use the correct line endings for 
	the target platform.
	
*   You can adapt the download and organization steps to your specific needs.

*   The provided URLs and version numbers for dependencies are based on your project's current 
configuration. If dependencies are updated in the Git Repository, Repeat this download/build procedure 
to get the latest changes. 


