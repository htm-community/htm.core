# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2013-2024, Numenta, Inc.
#   Migrated to scikit-build-core:  David Keeney, dkeeney@gmail.com, Dec 2024
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero Public License (http://www.gnu.org/licenses) for more details.
#
# You should have received a copy of the GNU Affero Public License
# along with this program.  
# ----------------------------------------------------------------------
#  python -m wheel unpack dist/htm-2.2.0-cp313-cp313-win_amd64.whl
# htm_install.py
import subprocess
import os
import glob
import shutil
import sys
import contextlib
import re

def main():
    # Check Python version
    if sys.version_info < (3, 9):
        print("Error: This project requires Python 3.9 or later.")
        print("You are running Python {}.{}.".format(sys.version_info.major, sys.version_info.minor))
        sys.exit(1)
        
    # Check if running in a virtual environment
    if not is_running_in_docker() and not in_venv():
      print("Error: Not running in a python virtual environment.")
      print("Please create a virtual environment before running this script.")
      print("You can create one using:")
      print("  python -m venv .venv")
      print("And activate it using:")
      print("  .venv\\Scripts\\activate  (Windows)")
      print("  source .venv/bin/activate (Linux/macOS)")
      sys.exit(1)

            
    # Install build dependencies
    print("Installing build dependencies...")
    subprocess.run([sys.executable, '-m', 'pip', 'install', 
              '--upgrade', 'pip', 
              'build', 
              'setuptools', 
              'wheel', 
              'pybind11', 
              'packaging',
              'pytest', 
              'requests'], check=True)     
              
    # Get the project version and minimum Cmake version from pyproject.toml
    # Install toml if needed (for Python < 3.11)
    if sys.version_info < (3, 11):
        subprocess.run([sys.executable, '-m', 'pip', 'install', 'toml'], check=True)
        import toml
    else:
        import tomllib as toml
    with open("pyproject.toml", "rb") as f:
        pyproject = toml.load(f)
        
    project_version = pyproject["project"]["version"]
    print(f"Version: {project_version}")
        
    # Ensure CMake is installed and meets the minimum version requirement
    import re
    min_cmake_version = get_cmake_minimum_version()
    if not check_cmake_version(min_cmake_version):
        print(f"Installing CMake >={min_cmake_version} using pip...")
        subprocess.run([sys.executable, '-m', 'pip', 'install', f'cmake>={min_cmake_version}'], check=True)
         
    # if the htm_core library does not exist, go build it.   
    htm_core_lib_path = os.path.join("build", "Release", "lib")
    if not (os.path.exists(os.path.join(htm_core_lib_path, "htm_core.lib")) or
            os.path.exists(os.path.join(htm_core_lib_path, "libhtm_core.a"))) :

        # Build the C++ components with CMake
        print("Building C++ components...")
        shutil.rmtree("build/cmake", ignore_errors=True)  # Clear cache, Ignore errors if the directory doesn't exist
        shutil.rmtree("dist", ignore_errors=True)         # Clear wheels, Ignore errors if the directory doesn't exist
        

        # Build the C++ htm_core library
        cmake_command = [
            "cmake",
            "-S", ".",
            "-B", "build/cmake",
            "-DBINDING_BUILD=CPP_Only",
            f"-DPROJECT_VERSION1={project_version}"
        ]
        print("CMake command:", cmake_command)  # Print the command before executing it
        subprocess.run(cmake_command, check=True, shell=False)
        
        cmake_command = [
            "cmake", 
            "--build", 
            "build/cmake", 
            "--config", 
            "Release", 
            "--target", 
            "install"]
        print("CMake command:", cmake_command)
        subprocess.run(cmake_command, check=True)
        print("C++ component build completed")
        print("")
        
    else:
        print("C++ components already built. Skipping C++ build...")

    wheel_file = find_wheel_file(project_version)
    if wheel_file == None:
        # Build the Python package with scikit-build-core
        print("Building Python package...")
    
        cmake_command = [sys.executable, "-m", "build"]
        print("CMake command:", cmake_command)
        subprocess.run(cmake_command, check=True)
        print("")
    else:
        print("Wheel already exists, skipping build of extensions...")
    
    # locate the .whl file we just created
    wheel_file = find_wheel_file(project_version)
    if wheel_file is None:
        print(f"Error: Could not find the wheel we just created in the 'dist' directory.")
        sys.exit(1)
       
    # Unpack the wheel (for testing)
    """
    print("Unpack the wheel...")
    subprocess.run([sys.executable, '-m', 'wheel', 'unpack', wheel_file], check=True)
    """
        
    # Install the package in Python.
    print("Installing the wheel...")
    cmake_command = [sys.executable, '-m', 'pip', 'install', '--force-reinstall', wheel_file]
    print("CMake command:", cmake_command)
    subprocess.run(cmake_command, check=True)
    
    print('Installation complete!')
    

    
def in_venv() -> bool:
    """Determine whether Python is running from a venv."""
    import sys
    if hasattr(sys, 'real_prefix'):
        return True
    pfx = getattr(sys, 'base_prefix', sys.prefix)
    return pfx != sys.prefix    
    
def is_running_in_docker():
    """ Checks if the script is running inside a Docker container. """
    try:
        with open('/proc/1/cgroup', 'r') as f:
            return 'docker' in f.read()
    except FileNotFoundError:
        return False  # Not Linux, so probably not Docker    
        
def get_cmake_minimum_version(cmake_file="CMakeLists.txt"):
    """Extracts the minimum required CMake version from a CMakeLists.txt file."""
    with open(cmake_file, "r") as f:
        for line in f:
            match = re.search(r"cmake_minimum_required\(VERSION\s*(\s*[\d.]+)", line)
            if match:
                return match.group(1)
    return None 
        
def check_cmake_version(min_version):
    """Checks the CMake version and returns True if it meets the minimum requirement."""
    from packaging import version
    try:
        result = subprocess.run(
            ["cmake", "--version"], capture_output=True, text=True, check=True
        )
        version_output = result.stdout
        version_str = version_output.splitlines()[0].split()[-1]
        
        # Parse the version string using packaging.version.Version
        installed_version = version.parse(version_str)
        required_version = version.parse(min_version)

        if installed_version >= required_version:
            print(f"Found CMake version {version_str}, which is sufficient.")
            return True
        else:
            print(f"CMake version {version_str} is too old. Requires {min_version}.")
            return False
    except FileNotFoundError:
        return False
    except subprocess.CalledProcessError as e:
        print(f"Error: CMake version check failed: {e}")
        sys.exit(1)
    except ValueError:
        print(f"Error: Could not parse CMake version: {version_str}")
        sys.exit(1)
        
def find_wheel_file(project_version):
    """Locates the wheel file in dist with matching Python and project versions."""
    wheel_file = None
    if os.path.exists('dist'):
        wheel_files = [f for f in os.listdir('dist') if f.startswith('htm-') and f.endswith('.whl')]
        pyver = f"cp{sys.version_info.major}{sys.version_info.minor}"
        pjver = f"htm-{project_version}"
        for whl in wheel_files:
            if pyver in whl and pjver in whl:
                wheel_file = os.path.join('dist', whl)
                break
    return wheel_file



if __name__ == "__main__":
    main()
