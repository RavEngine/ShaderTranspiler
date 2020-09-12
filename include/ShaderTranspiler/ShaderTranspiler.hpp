#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <Public/ShaderLang.h>

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
	CompileTask(const std::filesystem::path& p, ShaderStage s): filename(p), stage(s){}
};

struct CompileResult{
	std::string data;
	bool isBinary;
	std::string suffix;
};

struct Options{
	bool mobile;
	uint32_t version;
};

class ShaderTranspiler{
protected:
	typedef std::vector<uint32_t> spirvbytes;
	static bool glslAngInitialized;
    
    /**
     * Factory
     * @return instance of DefaultTBuiltInResource struct with appropriate fields set
     */
    static TBuiltInResource CreateDefaultTBuiltInResource();
	
	/**
	 Compile GLSL to SPIR-V bytes
	 @param filename the file to compile
	 @param ShaderType the type of shader to compile
	 */
	const std::vector<uint32_t> CompileGLSL(const std::filesystem::path& filename, const EShLanguage ShaderType);
	
    /**
     Decompile SPIR-V to OpenGL ES shader
     @param bin the SPIR-V binary to decompile
     @return OpenGL-ES source code
     */
	std::string SPIRVToESSL(const spirvbytes& bin, const Options& options);
    
    /**
     Decompile SPIR-V to DirectX shader
     @param bin the SPIR-V binary to decompile
     @return HLSL source code
     */
	std::string SPIRVToHLSL(const spirvbytes& bin, const Options& options);
    
    /**
     Decompile SPIR-V to Metal shader
     @param bin the SPIR-V binary to decompile
     @param mobile set to True to compile for Apple Mobile platforms
     @return Metal shader source code
     */
	std::string SPIRVtoMSL(const spirvbytes& bin, const Options& options);
	
	/**
	 Perform standard optimizations on a SPIR-V binary
	 @param bin the SPIR-V binary to optimize
	 @param options settings for the optimizer
	 */
	spirvbytes OptimizeSPIRV(const spirvbytes& bin, const Options& options);
	
	/**
	 Serialize a SPIR-V binary
	 @param bin the binary
	 */
	CompileResult SerializeSPIRV(const spirvbytes& bin);
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
