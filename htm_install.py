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
import tomllib

def main():
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
    subprocess.run([sys.executable, '-m', 'pip', 'install', '--upgrade', 'pip', 'build', 'setuptools', 'wheel', 'pybind11', 'pytest', 'requests'], check=True)     
    
    
    # Check if the math extension module exists.  If it does then the rest of them probably exist.
    #math_module_path = os.path.join("build", "htm", "bindings", "math.*")  # Use wildcard for platform independence
    #print(f"math_module_path = {math_module_path}")
    #if not any(os.path.exists(f) for f in glob.glob(math_module_path)):
    
    htm_core_lib_path = os.path.join("build", "Release", "lib", "htm_core.*")
    if not any(os.path.exists(f)  for f in glob.glob(htm_core_lib_path)):
        # Build the C++ components with CMake
        print("Building C++ components...")
        #shutil.rmtree("build/cmake", ignore_errors=True)  # Clear cache, Ignore errors if the directory doesn't exist

        # Get the pybind11 installation directory using pip
        result = subprocess.run([sys.executable, "-m", "pip", "show", "pybind11"], capture_output=True, text=True, check=True)
        # Split the output into lines and find the line containing "Location:"
        for line in result.stdout.splitlines():
            if "Location:" in line:
                # Find the index of the first colon
                colon_index = line.index(":")  
                # Extract the path after the first colon
                pybind11_location = line[colon_index + 1:].strip()
                break
        else:
            raise RuntimeError("Could not find pybind11 location")
        pybind11_dir = os.path.join(pybind11_location, "pybind11", "share", "cmake", "pybind11")
        
        # Read the version from pyproject.toml
        with open("pyproject.toml", "rb") as f:
            pyproject = tomllib.load(f)
        project_version = pyproject["project"]["version"]
        print(f"Version: {project_version}")

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
        
    else:
        print("C++ components already built. Skipping C++ build...")

    # Build the Python package with scikit-build-core
    print("Building Python package...")
    
    cmake_command = [sys.executable, "-m", "build"]
    print("CMake command:", cmake_command)
    subprocess.run(cmake_command, check=True)
    wheel_files = [f for f in os.listdir('dist') if f.startswith('htm-') and f.endswith('.whl')]
    wheel_file = os.path.join('dist', wheel_files[0])  # Use the first found wheel file
       
    # Unpack the wheel (for testing)
    """
    print("Unpack the wheel...")
    subprocess.run([sys.executable, '-m', 'wheel', 'unpack', wheel_file], check=True)
    """
        
    # Install the package in Python.
    print("Installing the wheel...")
    cmake_command = [sys.executable, '-m', 'pip', 'install', wheel_file]
    print("CMake command:", cmake_command)
    subprocess.run(cmake_command, check=True)
    
    # Determine the unpacked directory name
    """
    unpacked_dir = os.path.splitext(os.path.basename(wheel_file))[0]
    unpacked_dir = '-'.join(unpacked_dir.split('-', 2)[:2])  # Split at the second hyphen
    print("unpacked_dir = ", unpacked_dir)

    # Clean up unpacked directory ... 
    try:
        #shutil.rmtree(unpacked_dir)  # keep this for debugging the build. But do not check-in with this directory.
        print("")
    except OSError as e:
        print(f"Error: Failed to remove unpacked directory '{unpacked_dir}': {e}")
    """
    print('Installation complete!')
    
@contextlib.contextmanager
def pushd(new_dir):
    """
    Context manager for temporarily changing the current working directory.
    """
    previous_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)    
        

    
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
        


if __name__ == "__main__":
    main()