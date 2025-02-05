default: test

fetch:
	bazel sync --repository_cache="./thirdparty"

release:
	bazel build -c opt --distdir=./thirdparty //...

debug:
	bazel build --distdir=./thirdparty //...

test_all: test example

test:
	bazel test -c opt --distdir=./thirdparty //...

example: toy_psi toy_mc_psi mc_psi

toy_psi:
	bazel run -c opt --distdir=./thirdparty //mcpsi/example:toy_psi

toy_mc_psi:
	bazel run -c opt --distdir=./thirdparty //mcpsi/example:toy_mc_psi

mc_psi:
	bazel run -c opt --distdir=./thirdparty //mcpsi/example:mc_psi

clean:
	bazel clean --expunge
	rm -rf bazel-*