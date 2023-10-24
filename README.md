# McPSI

McPSI is stand for Malicious Circuit Private Set Intersection.

The project depends on [YACL](https://github.com/secretflow/yacl).

（Still in **developing stage**!!! **Did not finish**）

## File layout:
+ [context](test/context/): provide runtime environment
+ [cr](test/cr/): correlated-randomness (e.g. Beaver Triple) 
+ [ss](test/ss/): SPDZ-like protocol, supports several operators (e.g. Mul, Shuffle) between public value and arithmetic share.
+ [utils](test/utils/): basic tools (e.g. 64bit field)

## Dependence

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

## Build && Test
```sh
bazel build //...
bazel test //...
```