# McPSI

McPSI is short for Malicious Circuit Private Set Intersection.

The project depends on [YACL](https://github.com/secretflow/yacl), which provide several cryptographic interface (e.g. prg, ot, network).


### File layout:
+ [context](mcpsi/context/): provide runtime environment
+ [cr](mcpsi/cr/): correlated-randomness (e.g. Beaver Triple, MAC generation) 
+ [ss](mcpsi/ss/): SPDZ-like protocol, supports several operators (e.g. Mul, Shuffle) between public value and arithmetic share.
+ [utils](mcpsi/utils/): basic tools (e.g. 64bit / 128bit field)

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
bazel run -c opt //mcpsi/cr:cr_test # test corelated randomness
bazel run -c opt //mcpsi/cr/utils:liner_code_test # test local linear code over field
bazel run -c opt //mcpsi/cr/utils:vole_test # test vole over field
bazel run -c opt //mcpsi/cr/utils:ot_adapter_test # test ot adapter
bazel run -c opt //mcpsi/cr/utils:vole_adapter_test # test vole adapter
bazel run -c opt //mcpsi/cr/utils:ot_helper_test # test ot helper (generating Beaver Triple && Base Vole)
bazel run -c opt //mcpsi/ss:public_test # test public (operation between PP)
bazel run -c opt //mcpsi/ss:ashare_test # test a-share (operation between AA,AP,PA) 
bazel run -c opt //mcpsi/ss:gshare_test # test g-share (DY-PRF)
```

simple example (toy psi)
```sh
bazel run -c opt //mcpsi/example:toy_psi # run toy psi // PoC
bazel run -c opt //mcpsi/example:toy_mc_psi # run toy circuit psi (sum) // PoC
bazel run -c opt //mcpsi/example:mc_psi -- --set0 size_of_set0 --set1 size_of_set1 --interset size_of_interset --CR 0/1 --cache 0/1 --thread thread_num # run malicious circuit psi (CR=1 for real cr and CR=0 for fake cr)(cache=1 for pre-computing correlated-randomness and cache=0 for generating correlated-randomness when needed)
```

mcpsi under socket network
```sh
bazel run -c opt //mcpsi/example:mc_psi -- --mode 1 --rank 0 --set0 size_of_set0 --set1 size_of_set1 --interset size_of_interset --CR 0/1 --thread thread_num # run malicious circuit psi for party 0
bazel run -c opt //mcpsi/example:mc_psi -- --mode 1 --rank 1 --set0 size_of_set0 --set1 size_of_set1 --interset size_of_interset --CR 0/1  --thread thread_num # run malicious circuit psi for party 1
```

command line flags
```sh
--mode 0/1 (default is 0)   --> 0 for memory mode, while 1 for socket network 
--rank 0/1                  --> 0 for party0, while 1 for party1 (memory mode would ignore this flag)
--set0 size_of_set0         --> input size of party0 (default 10000)
--set1 size_of_set1         --> input size of party1 (default 10000)
--interset size_of_interset --> the size of intersect (default 100)
--CR 0/1                    --> 0 for fake correlation randomness (use PRG to simulate offline randomness), while 1 for true correlation randomness (use OT and VOLE to generate offline randomness)
--cache 0/1                 --> 0 for NO offline/online separating, generating CR when online is needed, while 1 for generating offline randomness before executing the online protocol.
--fairness 0/1              --> 0 for normal OPRF, while 1 for fair OPRF
--thread thread_num         --> number of threads for each party (default 1)
```

### Abort Dockerfile
```sh
# build docker image
docker build -t mcpsi:latest .   
docker run -it --name mcpsi-dev --cap-add=NET_ADMIN --privileged=true mcpsi:latest bash

# re-enter it or stop it
docker start mcpsi-dev          # start 
docker exec -it mcpsi-dev bash  # launch the terminal
docker stop mcpsi-dev           # stop
```
