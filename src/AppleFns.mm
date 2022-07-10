#include "ShaderTranspiler.hpp"
#include <SPIRV/GlslangToSpv.h>
#include <spirv_msl.hpp>
#include <spirv-tools/optimizer.hpp>

#import <Foundation/NSTask.h>
#import <Foundation/Foundation.h>

using namespace shadert;

extern IMResult SPIRVtoMSL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model);

struct executeProcessResult{
	int code = 0;
	NSData *outdata, *errdata;
};
executeProcessResult executeProcess(NSString* launchPath, NSArray<NSString*>* arguments, NSData* stdinData){
	// create task and pipes
	NSTask* task = [NSTask new];
	NSPipe* taskStdin = [NSPipe new];
	NSPipe* taskStdout = [NSPipe new];
	NSPipe* taskStderr = [NSPipe new];
	
	// connect pipes
	[task setStandardInput:taskStdin];
	[task setStandardOutput:taskStdout];
	[task setStandardError:taskStderr];
	task.currentDirectoryPath = [[NSFileManager defaultManager] currentDirectoryPath];
	
	[task setLaunchPath:launchPath];
	[task setArguments:arguments];
	
	// execute the command
	[task launch];
	// provide it with its data
	[[taskStdin fileHandleForWriting] writeData:stdinData];
	[[taskStdin fileHandleForWriting] closeFile];	// this tells xcrun that it can start reading from the pipe
	[task waitUntilExit];
	
	NSData* outdata = [[taskStdout fileHandleForReading] readDataToEndOfFile];
	NSData* errdata = [[taskStderr fileHandleForReading] readDataToEndOfFile];
	
	[[taskStdout fileHandleForReading] closeFile];
	[[taskStderr fileHandleForReading] closeFile];
	
	[taskStdin dealloc];
	[taskStdout dealloc];
	[taskStderr dealloc];
	
	// return exit code and the stdout/stderr data
	return {
		[task terminationStatus],
		outdata, errdata
	};
}

IMResult SPIRVtoMBL(const spirvbytes& bin, const Options& opt, spv::ExecutionModel model){
	// first make metal source shader
	auto MSLResult = SPIRVtoMSL(bin, opt, model);
	
	// create the AIR
	// the "-" argument tells it to read from stdin
	auto airResult = executeProcess(@"/usr/bin/xcrun",@[@"-sdk",@"macosx",@"metal",@"-c",@"-x",@"metal",@"-",@"-o",@"/dev/stdout"],[NSData dataWithBytes:MSLResult.sourceData.data() length:MSLResult.sourceData.size()]);
	
	if (airResult.code == 0){
		// now need to do it again to make the metallib from the AIR
		auto mtllibResult = executeProcess(@"/usr/bin/xcrun", @[@"-sdk",@"macosx",@"metallib",@"-",@"-o",@"/dev/stdout"], airResult.outdata);
		if (mtllibResult.code == 0){
			using binary_t = decltype(MSLResult.binaryData)::value_type;
			
			// this time the output is the final metallib
			MSLResult.binaryData.assign((binary_t*)mtllibResult.outdata.bytes, (binary_t*)mtllibResult.outdata.bytes + mtllibResult.outdata.length);
			return MSLResult;
		}
		else{
			throw std::runtime_error((const char*)mtllibResult.errdata.bytes);
		}
		
	}
	else{
		throw std::runtime_error((const char*)airResult.errdata.bytes);
	}
}
