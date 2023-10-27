workspace(name = "test")

load("//bazel:repositories.bzl", "mcpsi_deps")

mcpsi_deps()

load("@yacl//bazel:repositories.bzl", "yacl_deps")

yacl_deps()

load(
    "@rules_foreign_cc//foreign_cc:repositories.bzl",
    "rules_foreign_cc_dependencies",
)

rules_foreign_cc_dependencies(
    register_built_tools = False,
    register_default_tools = False,
    register_preinstalled_tools = True,
)
