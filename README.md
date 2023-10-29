# McPSI

McPSI is short for Malicious Circuit Private Set Intersection.

The project depends on [YACL](https://github.com/secretflow/yacl), which provide several cryptographic interface (e.g. prg, ot, network).

（Still in **developing stage**!!!）

### File layout:
+ [context](test/context/): provide runtime environment
+ [cr](test/cr/): correlated-randomness (e.g. Beaver Triple) 
+ [ss](test/ss/): SPDZ-like protocol, supports several operators (e.g. Mul, Shuffle) between public value and arithmetic share.
+ [utils](test/utils/): basic tools (e.g. 64bit field)

### Dependencies

#### Linux
```sh
Install gcc>=10.3, cmake, ninja, nasm
```

#### macOS
```sh
# Install Xcode
https://apps.apple.com/us/app/xcode/id497799835?mt=12

# Select Xcode toolchain version
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

# Install homebrew
https://brew.sh/

# Install dependencies
brew install bazel cmake ninja nasm automake libtool
```

### Build && Test

debug mode (only for developing)
```sh
bazel build //... # compile all files
bazel test //... # run all test
```

performance mode
```sh
bazel build -c opt //... # compile all files (with -O2)
bazel test -c opt //... # run all test (with -O2)
```

test components
```sh
bazel run -c opt //mcpsi/utils:field_test # test field operation 
bazel run -c opt //mcpsi/utils:vec_op_test # test vec field operation 
bazel run -c opt //mcpsi/context:context_test # test context 
bazel run -c opt //mcpsi/ss:public_test # test public (operation between PP)
bazel run -c opt //mcpsi/ss:ashare_test # test a-share (operation between AA,AP,PA) 
bazel run -c opt //mcpsi/ss:gshare_test # test g-share (DY-PRF)
```

simple example (toy psi)
```sh
bazel run -c opt //mcpsi/example:toy_psi # run toy psi // PoC
bazel run -c opt //mcpsi/example:toy_mc_psi # run toy circuit psi (sum) // PoC
bazel run -c opt //mcpsi/example:mc_psi -- --set0 size_of_set0 --set1 size_of_set1 --interset size_of_interset # run circuit psi
```
