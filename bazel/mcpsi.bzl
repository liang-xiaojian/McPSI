
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake", "configure_make")

WARNING_FLAGS = [
    "-Wall",
    "-Wextra",
    "-Werror",
]

# set `SPDLOG_ACTIVE_LEVEL=1(SPDLOG_LEVEL_DEBUG)` to enable debug level log
DEBUG_FLAGS = ["-DSPDLOG_ACTIVE_LEVEL=1", "-O0", "-g"]
RELEASE_FLAGS = ["-O2"]
FAST_FLAGS = ["-O1"]

AES_COPT_FLAGS = select({
    "@platforms//cpu:aarch64": ["-O3"],
    "//conditions:default": [
        "-mavx",
        "-maes",
    ],
})

def _mcpsi_copts():
    return select({
        "@test//bazel:mcpsi_build_as_release": RELEASE_FLAGS,
        "@test//bazel:mcpsi_build_as_debug": DEBUG_FLAGS,
        "@test//bazel:mcpsi_build_as_fast": FAST_FLAGS,
        "//conditions:default": FAST_FLAGS,
    }) + WARNING_FLAGS

def mcpsi_cc_binary(
        linkopts = [],
        copts = [],
        **kargs):
    cc_binary(
        linkopts = linkopts + ["-lm"],
        copts = copts + _mcpsi_copts(),
        **kargs
    )

def mcpsi_cc_library(
        linkopts = [],
        copts = [],
        deps = [],
        **kargs):
    cc_library(
        linkopts = linkopts,
        copts = _mcpsi_copts() + copts,
        deps = deps + [
            "@com_github_gabime_spdlog//:spdlog",
        ],
        **kargs
    )

def mcpsi_cmake_external(**attrs):
    if "generate_args" not in attrs:
        attrs["generate_args"] = ["-GNinja"]
    return cmake(**attrs)

def mcpsi_configure_make(**attrs):
    if "args" not in attrs:
        attrs["args"] = ["-j 4"]
    return configure_make(**attrs)

def mcpsi_cc_test(
        linkopts = [],
        copts = [],
        deps = [],
        linkstatic = True,
        **kwargs):
    cc_test(
        # -lm for tcmalloc
        linkopts = linkopts + ["-lm"],
        copts = _mcpsi_copts() + copts,
        deps = deps + [
            "@com_google_googletest//:gtest_main",
        ],
        # static link for tcmalloc
        linkstatic = True,
        **kwargs
    )
