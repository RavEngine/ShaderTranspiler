#include <ShaderTranspiler.hpp>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <filesystem>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#include <spirv-tools/optimizer.hpp>
#include <spirv_reflect.hpp>
#include <iostream>
#include <sstream>

using namespace std;
using namespace std::filesystem;
using namespace shadert;

static bool glslAngInitialized = false;

typedef std::vector<uint32_t> spirvbytes;

ReflectData::Resource::Resource(const spirv_cross::Resource& other) : id(other.id), type_id(other.type_id), base_type_id(other.base_type_id), name(std::move(other.name)){}

static ReflectData getReflectData(const spirv_cross::Compiler& comp){
	auto rsc = comp.get_shader_resources();
	
	
	ReflectData refl{
		.uniform_buffers{rsc.uniform_buffers.data(),rsc.uniform_buffers.begin() + rsc.uniform_buffers.size()},
		.storage_buffers{rsc.storage_buffers.data(),rsc.storage_buffers.begin() + rsc.storage_buffers.size()},
		.stage_inputs{rsc.stage_inputs.data(),rsc.stage_inputs.begin() + rsc.stage_inputs.size()},
		.stage_outputs{rsc.stage_outputs.data(),rsc.stage_outputs.begin() + rsc.stage_outputs.size()},
		.subpass_inputs{rsc.subpass_inputs.data(),rsc.subpass_inputs.begin() + rsc.subpass_inputs.size()},
		.storage_images{rsc.storage_images.data(),rsc.storage_images.begin() + rsc.storage_images.size()},
		.sampled_images{rsc.sampled_images.data(),rsc.sampled_images.begin() + rsc.sampled_images.size()},
		.atomic_counters{rsc.atomic_counters.data(),rsc.atomic_counters.begin() + rsc.atomic_counters.size()},
		.acceleration_structures{rsc.acceleration_structures.data(),rsc.acceleration_structures.begin() + rsc.acceleration_structures.size()},
		.push_constant_buffers{rsc.push_constant_buffers.data(),rsc.push_constant_buffers.begin() + rsc.push_constant_buffers.size()},
		.separate_images{rsc.separate_images.data(),rsc.separate_images.begin() + rsc.separate_images.size()},
		.separate_samplers{rsc.separate_samplers.data(),rsc.separate_samplers.begin() + rsc.separate_samplers.size()},
	};
	
	
	return refl;
}


/**
* Set the name of the main function 
* @param compiler the compiler instance to use
* @param entrypoint_name the new name for the shader entrypoint function
*/
template<typename T>
static void setEntryPoint(T& compiler, const std::string& entrypoint_name) {
	auto entryPoints = compiler.get_entry_points_and_stages();
	if (!entryPoints.empty()) {
		compiler.rename_entry_point(entryPoints[0].name, entrypoint_name, entryPoints[0].execution_model);
	}
}

/**
 * Factory
 * see https://github.com/ForestCSharp/VkCppRenderer/blob/master/Src/Renderer/GLSL/ShaderCompiler.hpp for more info
 * @return instance of DefaultTBuiltInResource struct with appropriate fields set
 */
TBuiltInResource CreateDefaultTBuiltInResource(){
    return TBuiltInResource{
		.maxLights = 32,
		.maxClipPlanes = 6,
		.maxTextureUnits = 32,
		.maxTextureCoords = 32,
		.maxVertexAttribs = 64,
		.maxVertexUniformComponents = 4096,
		.maxVaryingFloats = 64,
		.maxVertexTextureImageUnits = 32,
		.maxCombinedTextureImageUnits = 80,
		.maxTextureImageUnits = 32,
		.maxFragmentUniformComponents = 4096,
		.maxDrawBuffers = 32,
		.maxVertexUniformVectors = 128,
		.maxVaryingVectors = 8,
		.maxFragmentUniformVectors = 16,
		 .maxVertexOutputVectors = 16,
		 .maxFragmentInputVectors = 15,
		 .minProgramTexelOffset = -8,
		 .maxProgramTexelOffset = 7,
		 .maxClipDistances = 8,
		 .maxComputeWorkGroupCountX = 65535,
		 .maxComputeWorkGroupCountY = 65535,
		 .maxComputeWorkGroupCountZ = 65535,
		 .maxComputeWorkGroupSizeX = 1024,
		 .maxComputeWorkGroupSizeY = 1024,
		 .maxComputeWorkGroupSizeZ = 64,
		 .maxComputeUniformComponents = 1024,
		 .maxComputeTextureImageUnits = 16,
		 .maxComputeImageUniforms = 8,
		 .maxComputeAtomicCounters = 8,
		 .maxComputeAtomicCounterBuffers = 1,
		 .maxVaryingComponents = 60,
		 .maxVertexOutputComponents = 64,
		 .maxGeometryInputComponents = 64,
		 .maxGeometryOutputComponents = 128,
		 .maxFragmentInputComponents = 128,
		 .maxImageUnits = 8,
		 .maxCombinedImageUnitsAndFragmentOutputs = 8,
		 .maxCombinedShaderOutputResources = 8,
		 .maxImageSamples = 0,
		 .maxVertexImageUniforms = 0,
		 .maxTessControlImageUniforms = 0,
		 .maxTessEvaluationImageUniforms = 0,
		 .maxGeometryImageUniforms = 0,
		 .maxFragmentImageUniforms = 8,
		 .maxCombinedImageUniforms = 8,
		 .maxGeometryTextureImageUnits = 16,
		 .maxGeometryOutputVertices = 256,
		 .maxGeometryTotalOutputComponents = 1024,
		 .maxGeometryUniformComponents = 1024,
		 .maxGeometryVaryingComponents = 64,
		 .maxTessControlInputComponents = 128,
		 .maxTessControlOutputComponents = 128,
		 .maxTessControlTextureImageUnits = 16,
		 .maxTessControlUniformComponents = 1024,
		 .maxTessControlTotalOutputComponents = 4096,
		 .maxTessEvaluationInputComponents = 128,
		 .maxTessEvaluationOutputComponents = 128,
		 .maxTessEvaluationTextureImageUnits = 16,
		 .maxTessEvaluationUniformComponents = 1024,
		 .maxTessPatchComponents = 120,
		 .maxPatchVertices = 32,
		 .maxTessGenLevel = 64,
		 .maxViewports = 16,
		 .maxVertexAtomicCounters = 0,
		 .maxTessControlAtomicCounters = 0,
		 .maxTessEvaluationAtomicCounters = 0,
		 .maxGeometryAtomicCounters = 0,
		 .maxFragmentAtomicCounters = 8,
		 .maxCombinedAtomicCounters = 8,
		 .maxAtomicCounterBindings = 1,
		 .maxVertexAtomicCounterBuffers = 0,
		 .maxTessControlAtomicCounterBuffers = 0,
		 .maxTessEvaluationAtomicCounterBuffers = 0,
		 .maxGeometryAtomicCounterBuffers = 0,
		 .maxFragmentAtomicCounterBuffers = 1,
		 .maxCombinedAtomicCounterBuffers = 1,
		 .maxAtomicCounterBufferSize = 16384,
		 .maxTransformFeedbackBuffers = 4,
		 .maxTransformFeedbackInterleavedComponents = 64,
		 .maxCullDistances = 8,
		 .maxCombinedClipAndCullDistances = 8,
		 .maxSamples = 4,
		 .limits = {
			 .nonInductiveForLoops = 1,
			 .whileLoops = 1,
			 .doWhileLoops = 1,
			 .generalUniformIndexing = 1,
			 .generalAttributeMatrixVectorIndexing = 1,
			 .generalVaryingIndexing = 1,
			 .generalSamplerIndexing = 1,
			 .generalVariableIndexing = 1,
			 .generalConstantMatrixVectorIndexing = 1,
		}
	};
}

struct CompileGLSLResult {
	spirvbytes spirvdata;
	std::vector<Uniform> uniforms;
};

const CompileGLSLResult CompileGLSL(const std::string_view& source, const EShLanguage ShaderType, const std::vector<std::filesystem::path>& includePaths) {
	//initialize. Do only once per process!
	if (!glslAngInitialized)
	{
		glslang::InitializeProcess();
		glslAngInitialized = true;
	}

	const char* InputCString = source.data();

	//determine the stage
	glslang::TShader Shader(ShaderType);

	//set the associated strings (in this case one, but shader meta JSON can describe more. Pass as a C array and a size.
	Shader.setStrings(&InputCString, 1);

	//=========== vulkan versioning (should alow this to be passed in, or find out from the system) ========
	const int DefaultVersion = 130;

	int ClientInputSemanticsVersion = DefaultVersion; // maps to, say, #define VULKAN 100
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_2;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;

	Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

	auto DefaultTBuiltInResource = CreateDefaultTBuiltInResource();

	TBuiltInResource Resources(DefaultTBuiltInResource);
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);



	// =============================== preprocess GLSL =============================
	DirStackFileIncluder Includer;

	//Get Path of File
	for (const auto& path : includePaths) {
		Includer.pushExternalLocalDirectory(path.string());
	}

	std::string PreprocessedGLSL;

	if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		string msg = string("GLSL Preprocessing failed: ") + Shader.getInfoLog() + "\n" + Shader.getInfoDebugLog();
		throw std::runtime_error(msg);
	}

	// update the stored strings (is the original set necessary?)
	const char* PreprocessedCStr = PreprocessedGLSL.c_str();
	Shader.setStrings(&PreprocessedCStr, 1);

	// ================ now parse the shader ================
	if (!Shader.parse(&Resources, DefaultVersion, false, messages))
	{
		string msg = string("GLSL Parsing failed: ") + Shader.getInfoLog() + "\n" + Shader.getInfoDebugLog();
		throw std::runtime_error(msg);
	}

	// ============== pass parsed shader and link it ==============
	glslang::TProgram Program;
	Program.addShader(&Shader);

	if (!Program.link(messages))
	{
		std::string msg = string("GLSL Linking failed") + Shader.getInfoLog() + "\n" + Shader.getInfoDebugLog();
		throw std::runtime_error(msg);
	}

	CompileGLSLResult result;

	// ========= convert to spir-v ============

	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), result.spirvdata, &logger, &spvOptions);

	// get uniform information
	Program.buildReflection();
	auto nUniforms = Program.getNumLiveUniformVariables();
	for (int i = 0; i < nUniforms; i++) {
		Uniform uniform;
		uniform.name = std::move(Program.getUniformName(i));
		uniform.arraySize = Program.getUniformArraySize(i);
		uniform.bufferOffset = Program.getUniformBufferOffset(i);
		uniform.glDefineType = Program.getUniformType(i);
		result.uniforms.push_back(std::move(uniform));
	}

	return result;
}

/**
 Compile GLSL to SPIR-V bytes
 @param filename the file to compile
 @param ShaderType the type of shader to compile
 */
const CompileGLSLResult CompileGLSLFromFile(const FileCompileTask& task, const EShLanguage ShaderType){
	
	
	//Load GLSL into a string
	std::ifstream file(task.filename);
	
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + task.filename.string());
	}
		
	//read input file into string, convert to C string
	std::string InputGLSL((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
	
	// add current directory
	std::vector<std::filesystem::path> pathsWithParent(std::move(task.includePaths));
	pathsWithParent.push_back(task.filename.parent_path());
	return CompileGLSL(InputGLSL, ShaderType, pathsWithParent);
}

/**
 Decompile SPIR-V to OpenGL ES shader
 @param bin the SPIR-V binary to decompile
 @return OpenGL-ES source code
 */
IMResult SPIRVToESSL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model){
	spirv_cross::CompilerGLSL glsl(std::move(bin));
		
	//set options
	spirv_cross::CompilerGLSL::Options options;
	options.version = opt.version;
	options.es = opt.mobile;
	glsl.set_common_options(options);

	setEntryPoint(glsl, opt.entryPoint);

	return {glsl.compile(), getReflectData(glsl)};
}

/**
 Decompile SPIR-V to DirectX shader
 @param bin the SPIR-V binary to decompile
 @return HLSL source code
 */
IMResult SPIRVToHLSL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model){
	spirv_cross::CompilerHLSL hlsl(std::move(bin));
	
	spirv_cross::CompilerHLSL::Options options;
	hlsl.set_hlsl_options(options);

	setEntryPoint(hlsl, opt.entryPoint);

	return {hlsl.compile(), getReflectData(hlsl)};
}

/**
 Decompile SPIR-V to Metal shader
 @param bin the SPIR-V binary to decompile
 @param mobile set to True to compile for Apple Mobile platforms
 @return Metal shader source code
 */
IMResult SPIRVtoMSL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model){
	spirv_cross::CompilerMSL msl(std::move(bin));
	
	spirv_cross::CompilerMSL::Options options;
	uint32_t major = opt.version / 10;
	uint32_t minor = opt.version % 10;
	options.set_msl_version(major,minor);
	options.platform = opt.mobile ? spirv_cross::CompilerMSL::Options::Platform::iOS : spirv_cross::CompilerMSL::Options::Platform::macOS;
	msl.set_msl_options(options);

	// TODO: see shaderc_metal.cpp:562 for setting resource bindings

	setEntryPoint(msl, opt.entryPoint);

	return {msl.compile(), getReflectData(msl)};
}

/**
 Serialize a SPIR-V binary
 @param bin the binary
 */
CompileResult SerializeSPIRV(const spirvbytes& bin){
	ostringstream buffer(ios::binary);
	buffer.write((char*)&bin[0], bin.size() * sizeof(uint32_t));
	return {{buffer.str()},true};
}

/**
 Perform standard optimizations on a SPIR-V binary
 @param bin the SPIR-V binary to optimize
 @param options settings for the optimizer
 */
spirvbytes OptimizeSPIRV(const spirvbytes& bin, const Options &options){
	
	spv_target_env target;
	switch(options.version){
		case 10:
			target = SPV_ENV_UNIVERSAL_1_0;
			break;
		case 11:
			target = SPV_ENV_UNIVERSAL_1_1;
			break;
		case 12:
			target = SPV_ENV_UNIVERSAL_1_2;
			break;
		case 13:
			target = SPV_ENV_UNIVERSAL_1_3;
			break;
		case 14:
			target = SPV_ENV_UNIVERSAL_1_4;
			break;
		case 15:
			target = SPV_ENV_UNIVERSAL_1_5;
			break;
		default:
			throw runtime_error("Unknown Vulkan version");
			break;
	}
	
	//create a general optimizer
	spvtools::MessageConsumer consumer = [&](spv_message_level_t level, const char* source, const spv_position_t& position, const char* message){
		switch(level){
			case SPV_MSG_FATAL:
			case SPV_MSG_INTERNAL_ERROR:
			case SPV_MSG_ERROR:
				throw runtime_error(message);
				break;

		}
	};
	spvtools::Optimizer optimizer(target);
	optimizer.RegisterSizePasses();
	optimizer.RegisterPerformancePasses();
	optimizer.SetMessageConsumer(consumer);
	
	spirvbytes newbin;
	if (optimizer.Run(bin.data(), bin.size(), &newbin)){
		return newbin;
	}
	else{
		throw runtime_error("Failed optimizing SPIR-V binary");
	}
}

struct APIConversion {
	EShLanguage type;
	spv::ExecutionModel model;
};
static APIConversion ShaderStageToInternal(ShaderStage api) {
	APIConversion cv;

	switch (api) {
	case ShaderStage::Vertex:
		cv.type = EShLangVertex;
		cv.model = decltype(cv.model)::ExecutionModelVertex;
		break;
	case ShaderStage::Fragment:
		cv.type = EShLangFragment;
		cv.model = decltype(cv.model)::ExecutionModelFragment;
		break;
	case ShaderStage::TessControl:
		cv.type = EShLangTessControl;
		cv.model = decltype(cv.model)::ExecutionModelTessellationControl;
		break;
	case ShaderStage::TessEval:
		cv.type = EShLangTessEvaluation;
		cv.model = decltype(cv.model)::ExecutionModelTessellationEvaluation;
		break;
	case ShaderStage::Geometry:
		cv.type = EShLangGeometry;
		cv.model = decltype(cv.model)::ExecutionModelGeometry;
		break;
	case ShaderStage::Compute:
		cv.type = EShLangCompute;
		cv.model = decltype(cv.model)::ExecutionModelGLCompute;
		break;
	}

	return cv;
}

static CompileResult CompileSpirVTo(const spirvbytes& spirv, TargetAPI api, const Options& opt,  APIConversion types) {
	switch (api) {
	case TargetAPI::OpenGL_ES:
		return CompileResult{ SPIRVToESSL(spirv,opt,types.model),false };
	case TargetAPI::OpenGL:
		break;
	case TargetAPI::Vulkan:
		return SerializeSPIRV(OptimizeSPIRV(spirv, opt));
		break;
	case TargetAPI::DirectX:
		return CompileResult{ SPIRVToHLSL(spirv,opt,types.model),false };
		break;
	case TargetAPI::Metal:
		return CompileResult{ SPIRVtoMSL(spirv,opt,types.model),false };
		break;
	default:
		throw runtime_error("Unsupported API");
		break;
	}
}

CompileResult ShaderTranspiler::CompileTo(const FileCompileTask& task, TargetAPI api, const Options& opt) {

	auto types = ShaderStageToInternal(task.stage);

	//generate spirv
	auto spirv = CompileGLSLFromFile(task, types.type);
	auto compres = CompileSpirVTo(spirv.spirvdata, api, opt, types);
	compres.data.uniformData = std::move(spirv.uniforms);
	return compres;
}

CompileResult ShaderTranspiler::CompileTo(const MemoryCompileTask& task, TargetAPI api, const Options& opt) {
	auto types = ShaderStageToInternal(task.stage);
	auto spirv = CompileGLSL(task.source, types.type, task.includePaths);
	auto compres = CompileSpirVTo(spirv.spirvdata, api, opt, types);
	compres.data.uniformData = std::move(spirv.uniforms);
	return compres;
}
