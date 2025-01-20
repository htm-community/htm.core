import os

def get_version():
    """Reads the version number from the VERSION file."""
    with open(os.path.join(os.path.dirname(__file__), 'VERSION'), 'r') as f:
        return f.read().strip()

if __name__ == "__main__":
    print(get_version()) 