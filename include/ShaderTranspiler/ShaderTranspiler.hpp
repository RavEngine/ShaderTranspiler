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


struct FileCompileTask{
	const std::filesystem::path& filename;
	const ShaderStage stage;
	const std::vector<std::filesystem::path> includePaths;	// optional
};

struct MemoryCompileTask {
	const std::string source;
	const ShaderStage stage;
	const std::vector<std::filesystem::path> includePaths;	// optional
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
    Execute the shader transpiler using shader source code in a file.
     @param task the CompileTask to execute. See CompileTask for information.
     @param platform the target API to compile to.
     @return A CompileResult representing the result of the compile.
     */
	CompileResult CompileTo(const FileCompileTask& task, const TargetAPI platform, const Options& options);

	/**
	Execute the shader transpiler using shader source code in memory.
	 @param task the CompileTask to execute. See CompileTask for information.
	 @param platform the target API to compile to.
	 @return A CompileResult representing the result of the compile.
	 */
	CompileResult CompileTo(const MemoryCompileTask& task, const TargetAPI platform, const Options& options);
};
}
