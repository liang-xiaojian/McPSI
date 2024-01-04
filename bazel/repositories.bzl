load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

SECRETFLOW_GIT = "https://github.com/secretflow"

YACL_COMMIT_ID = "5feaa30e6a2ab3be5a01a7a4ee3c1613d11386d9"

RULES_BOOST_COMMIT_ID = "1b6711875be9d90140e3c8e558667723bd4bed93"

SKYLIB_VERSION = "1.3.0"

def _yacl():
    maybe(
        git_repository,
        name = "yacl",
        commit = YACL_COMMIT_ID,
        remote = "https://github.com/secretflow/yacl.git",
    )

def _gmp():
    maybe(
        http_archive,
        name = "gmp",
        build_file = "//bazel:gmp.BUILD",
        # sha256 = "fd4829912cddd12f84181c3451cc752be224643e87fac497b69edddadc49b4f2",
        strip_prefix = "gmp-6.3.0",
        urls = ["https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz"],
    )

def _boost():
    maybe( 
        git_repository,
        name = "com_github_nelhage_rules_boost",
        commit = RULES_BOOST_COMMIT_ID,
        remote = "https://github.com/nelhage/rules_boost",
        shallow_since = "1580416893 -0800",
    )

def mcpsi_deps():
    _yacl()
   
    maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = SKYLIB_VERSION),
            "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = SKYLIB_VERSION),
        ],
    )

    _gmp()

    _boost()

    
