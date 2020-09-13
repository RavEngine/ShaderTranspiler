# ShaderTranspiler
A clean and simple C++ library to convert GLSL shaders to HLSL, Metal, and Vulkan. It leverages [glslang](https://github.com/KhronosGroup/glslang), 
[SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross), and [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) to handle cross-compilation of shaders. 

## Supported APIs:
- GLSL -> ESSL (OpenGL ES) 
- GLSL -> HLSL (DirectX)
- GLSL -> MSL (Metal, mobile and desktop)
- GLSL -> SPIR-V (Vulkan)


## How to use
```cpp
// include the library
#include <ShaderTranspiler/ShaderTranspiler.hpp>
#include <iostream>

using namespace std::filesystem;
using namespace std;

//the library's namespace
using namespace shadert;

int main(){

  //create an instance
  ShaderTranspiler s;

  //Create a CompileTask with the path to your shader and its stage.
  //The path is required because this library supports the OpenGL #include extension
  CompileTask task(path("Scene.vert"),ShaderStage::Vertex);

  //configure the compile with an Options object
  Options opt;
  opt.mobile = false; //used for OpenGL ES or Metal iOS
  opt.version = 15;   //stores the major and minor version, for Vulkan 1.5 use 15

  try{
    //call CompileTo and pass the CompileTask and the Options
    CompileResult result = s.CompileTo(task, TargetAPI::Vulkan, opt);

    //the shader data is stored in the data field
    cout << (result.isBinary? "Binary" : "Plain text") << "shader created, source = " << endl;
    cout << result.data << endl;
  }
  catch(exception& e){
    //library will throw on errors
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
```

## How to compile
This library uses CMake. Simply call `add_subdirectory` in your CMakeLists.txt.
```cmake
# ...
add_subdirectory("ShaderTranspiler")
# ...
target_link_libraries("your-program" PRIVATE "ShaderTranspiler")
```
If you simply want to play around with the library, you can use one of the init scripts (`init-mac.sh`, `init-win.sh`) and modify `main.cpp` in the test folder.

# Issue reporting
Known issues:
- Dependence on `std::filesystem`, C++17, and `/std:c++latest`
- Writing SPIR-V binaries on a Big Endian machine will not work.

Use the Issues section to report problems. Contributions are welcome, use pull requests. 
