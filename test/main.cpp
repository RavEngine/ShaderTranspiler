#include <ShaderTranspiler/ShaderTranspiler.hpp>
#include <iostream>

using namespace std::filesystem;
using namespace std;

int main(){
	ShaderTranspiler s;
	
	CompileTask task(path("Scene.vert"),ShaderStage::Vertex);
	auto result = s.CompileTo(task, TargetAPI::Vulkan, {false,15});
	
	cout << result.data << endl;
	
	return 0;
}
