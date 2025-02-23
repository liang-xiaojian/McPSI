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

# Quick Start
# you could change size of set in `.env`
run:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi -- --set0=$(SET0) --set1=$(SET1) --CR=$(CR) --cache=$(CACHE) --thread=$(THREAD) --fairness=$(FAIRNESS)

run_p0:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi -- --mode=1 --rank=0 --set0=$(SET0) --set1=$(SET1) --CR=$(CR) --cache=$(CACHE) --thread=$(THREAD) --fairness=$(FAIRNESS)

run_p1:
	bazel run -c opt --distdir=$(DISTDIR) --copt=$(COPT) --jobs=$(JOBS) //mcpsi/example:mc_psi -- --mode=1 --rank=1 --set0=$(SET0) --set1=$(SET1) --CR=$(CR) --cache=$(CACHE) --thread=$(THREAD) --fairness=$(FAIRNESS)

# network setting
# make sure you have already install `tc`
10M:
	tc qdisc del dev lo root
	tc qdisc add dev lo root handle 1:0 tbf rate 10Mbit latency 20ms burst 65536k
	tc qdisc add dev lo parent 1:0 handle 10:0 netem delay 20ms
100M:
	tc qdisc del dev lo root
	tc qdisc add dev lo root handle 1:0 tbf rate 100Mbit latency 20ms burst 65536k
	tc qdisc add dev lo parent 1:0 handle 10:0 netem delay 20ms
LAN:
	tc qdisc del dev lo root
	tc qdisc add dev lo root handle 1:0 tbf rate 1Gbit latency 20ms burst 65536k
LO:
	tc qdisc del dev lo root