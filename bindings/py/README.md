
## Layout of directories:
```
  REPO_DIR    external  -- cmake scripts to download/build dependancies
    bindings
        py   -- Location of Python bindings
          packaging -- Contains things needed to build package
          cpp_src 
            bindings  -- C++ pybind11 mapping source code 
            plugin    -- C++ code to manage python plugin
    py        htm           -- python source code goes here        tests         -- python unit tests here    src        htm           -- C++ source code for libraries        examples      -- C++ source code for examples        tests         -- C++ source code for tests    build             -- Where build results end up.        ThirdParty    -- where mnist data and all override thirdParty packages are installed.        Release       -- Where C++ build artifacts are installed
```

