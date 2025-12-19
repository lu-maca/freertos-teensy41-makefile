#!/usr/bin/env python3
# encoding: utf-8


from waflib import Logs

LIBNAME = "simcore"


# ******************************************************************************
# OPTIONS
# ******************************************************************************
def options(ctx):
    # Load GCC options
    ctx.load("g++")
    sc = ctx.add_option_group("SimCore options")


# ******************************************************************************
# CONFIGURE
# ******************************************************************************
def configure(ctx):
    Logs.pprint("BLUE", "\nConfiguring...")
    ctx.load("g++")

    # SETUP BUILD --------------------------------------------------------------
    # Generate build arguments
    ctx.env.append_unique("CXXFLAGS", ["-std=c++20", "-Os", "-Wall", "-g", "-pthread"])

    # Files
    ctx.env.append_unique("INCLUDES", [
        ctx.path.find_dir("simcore/inc/").abspath(), ctx.path.find_dir("simcore/inc/serialprotocol").abspath()
    ])
    ctx.env.append_unique("FILES", ["simcore/src/Component.cpp"])

    # LIBRARIES ----------------------------------------------------------------

    # Add nnxx (nanomsg next gen) library
    ctx.check_cc(lib="nnxx", mandatory=True)
    ctx.env.append_unique("LIBS", ["nnxx"])

    # Add msgpack library
    ctx.check_cc(lib="msgpack-c", mandatory=True)
    ctx.env.append_unique("LIBS", ["msgpack-c"])

    # Add reflectcpp library (for msgpack c++)
    ctx.check_cc(lib="reflectcpp", mandatory=True)
    ctx.env.append_unique("LIBS", ["reflectcpp"])

    # Add spdlog library (logger https://github.com/gabime/spdlog)
    ctx.check_cc(lib="spdlog", mandatory=True)
    ctx.env.append_unique("LIBS", ["spdlog"])

    # Add fmt library: spdlog depends on it (https://github.com/fmtlib/fmt)
    ctx.check_cc(lib="fmt", mandatory=True)
    ctx.env.append_unique("LIBS", ["fmt"])


# ******************************************************************************
# BUILD
# ******************************************************************************
def build(ctx):
    # build libsimcore.a
    Logs.pprint("BLUE", "\nBuilding lib{}...".format(LIBNAME))

    ctx.stlib(
        source=ctx.path.ant_glob(ctx.env.FILES),
        target=LIBNAME,
        name=LIBNAME,
        includes=ctx.env.INCLUDES,
        lib=ctx.env.LIBS,
        cxxflags=ctx.env.CFLAGS_cshlib,
    )
