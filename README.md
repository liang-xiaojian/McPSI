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
bazel build //...
bazel test //...
```

performance mode
```sh
bazel -c opt build //...
bazel -c opt test //...
```