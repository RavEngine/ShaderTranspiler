cd deps
cd SPIRV-Tools
rm -r android_test Android.mk build_defs.bzl BUILD.bazel BUILD.gn docs examples kokoro test

cd ../../../glslang
rm -r BUILD.bazel BUILD.gn gtests kokoro ndk_test Test _config.yml .appveyor.yml .clang-format .travis.yml Android.mk WORKSPACE

cd ../SPIRV-Reflect
rm -r examples common