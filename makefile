include .env

default: test

fetch:
	bazel sync --repository_cache=$(DISTDIR)

release:
	bazel build -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //...

debug:
	bazel build --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //...

test_all: test example

test:
	bazel test -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //...

example: toy_psi toy_mc_psi mc_psi

toy_psi:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:toy_psi

toy_mc_psi:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:toy_mc_psi

mc_psi:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi

clean:
	bazel clean --expunge
	rm -rf bazel-*

run:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi -- --set0=$(SET0) --set1=$(SET1) --CR=$(CR) --cache=$(CACHE) --thread=$(THREAD) --fairness=$(FAIRNESS)

run_p0:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi -- --mode=1 --rank=0 --set0=$(SET0) --set1=$(SET1) --CR=$(CR) --cache=$(CACHE) --thread=$(THREAD) --fairness=$(FAIRNESS)

run_p1:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi -- --mode=1 --rank=1 --set0=$(SET0) --set1=$(SET1) --CR=$(CR) --cache=$(CACHE) --thread=$(THREAD) --fairness=$(FAIRNESS)
