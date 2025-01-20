#    Print a nested directory listing
#
import os

def list_files_in_directory(directory):
  """
  Lists all files and subdirectories within the given directory.

  Args:
    directory: The path to the directory to list.

  Returns:
    A list of strings, where each string is the path to a file or subdirectory 
    relative to the given directory.
  """
  files_and_dirs = []
  for root, dirs, files in os.walk(directory):
    for file in files:
      files_and_dirs.append(os.path.join(os.path.relpath(root, directory), file))
    for dir_ in dirs:
      files_and_dirs.append(os.path.join(os.path.relpath(root, directory), dir_))
  return files_and_dirs

# Replace 'path/to/extracted/wheel' with the actual path to the extracted wheel directory
wheel_path = "htm-2.2.0" 

# Get the list of files and subdirectories within the extracted wheel
all_files = list_files_in_directory(wheel_path)

# Print the list of files and subdirectories
for file_or_dir in all_files:
  print(file_or_dir)