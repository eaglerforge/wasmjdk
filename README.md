# statically-linked wasm jdk
### lwjgl still very broken for now

## Compilation Instructions:
- Use a nixOS system/vm/container. This makes dependency management very easy.
  - If you know what you are doing, list of deps is [here](shell.nix)
- `git clone --depth=1 https://github.com/eaglerforge/wasmjdk.git`
- `cd wasmjdk`
- `chmod +x *.sh`
- `rm ./libffi/configure`
- `./build_ffi.sh`
- `./build_jvm.sh config`
- `./build_jvm.sh`
- Get LWJGL reference classes
  - In a browser, go to: `https://www.lwjgl.org/customize`
  - Select `linux x64` as the platform, and `minimal opengl es` as the preset
  - In the contents tab, remove `OpenAL` and `Assimp`
  - press download button, save to the `lwjgl/classpath` directory.
  - the `build_graphics.sh` script will handle the rest
- `./build_graphics.sh` (lwjgl + gl4es, you may see some errors, pretend they don't exist)
- `./build_jvm.sh skip` again, this time it will include lwjgl stuff
- Finally: `./build_runner.sh`, which will compile everything to a .html file


## Repo Structure:
- `libffi` - directory containing libffi and it's binaries
- `jdk` - the openjdk 25 port (hardcoded patches onto unix/linux targets)
- `wasmjdk_build` - build output for the raw wasm JVM/JNI library
  - `wasmjdk_build/include/` is just JNI headers
  - use `wasmjdk_build/monolith/libjvm.a` to link against. heads up, it will contain lwjgl objects by default if you build with that.
  - `wasmjdk_build/runtime/lib/modules`: the runtime modules file for the JVM
  - `wasmjdk_build/jmod`: Contains whichever .jmod files are enabled inside the `build_jvm.sh` script (you can edit `JMOD_TARGETS`)
- `RELEASE`: flag for compiling in debug vs release mode. 0 is debug, 1 is release
- `patch_include` - headerfiles for shimming unsupported platform features
- `runner` - the application that is built into the `docs/` directory
  - `runner/main.cpp` - contains the entry point, and patched `dlopen` (important if you want to use this for your own causes)
  - `runner/template/` - website template
  - `runner/test/` - contains java programs that are loaded into the website. Simply follow folder structure and you can add pretty much any java program.