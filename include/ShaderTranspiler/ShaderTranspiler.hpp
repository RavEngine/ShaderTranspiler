#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace shadert{
enum class ShaderStage{
	Vertex,
	Fragment,
	TessControl,
	TessEval,
	Geometry,
	Compute
};

enum class TargetAPI{
	OpenGL_ES = 0,
	OpenGL,
	Vulkan,
	DirectX11,
	Metal,
};

struct CompileTask{
	const std::filesystem::path& filename;
	const ShaderStage stage;
};

struct CompileResult{
	std::string data;
	bool isBinary;
};

struct Options{
	uint32_t version;
	bool mobile;
	std::string entryPoint = "frag";
};

class ShaderTranspiler{
public:
    /**
    Execute the shader transpiler.
     @param task the CompileTask to execute. See CompileTask for information.
     @param platform the target API to compile to.
     @return A CompileResult representing the result of the compile.
     */
	CompileResult CompileTo(const CompileTask& task, const TargetAPI platform, const Options& options);
};
}
