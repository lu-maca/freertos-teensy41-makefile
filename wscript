#!/usr/bin/env python
from waflib import Options, Configure, Build
from waflib import Context
import os
import glob

nano_specs = os.popen(
    'arm-none-eabi-gcc -print-file-name=nano.specs'
).read().strip()

# ---------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------
def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')

# ---------------------------------------------------------------------
# Configure
# ---------------------------------------------------------------------
def configure(cfg):
    cfg.load('compiler_c')
    cfg.load('compiler_cxx')

    cfg.env.CC        = 'arm-none-eabi-gcc'
    cfg.env.CXX       = 'arm-none-eabi-g++'
    cfg.env.AR        = 'arm-none-eabi-ar'
    cfg.env.OBJCOPY   = 'arm-none-eabi-objcopy'
    cfg.env.SIZE      = 'arm-none-eabi-size'

    # CRITICAL: force linkers
    cfg.env.LINK_CC   = cfg.env.CC
    cfg.env.LINK_CXX  = cfg.env.CXX

    cfg.env.MCU = 'IMXRT1062'
    cfg.env.MCU_DEF = 'ARDUINO_TEENSY41'
    cfg.env.LDSCRIPT = 'teensy4/imxrt1062_t41.ld'

# ---------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------
def build(bld):
    TARGET = bld.path.name

    COREPATH = 'teensy4'
    LIBPATH  = 'libs'
    SRCPATH  = 'src'

    FREERTOS_CPP_INC = 'libs/freertos/lib/cpp/include'

    # ---------------------------------------------------------------
    # Compiler flags
    # ---------------------------------------------------------------
    cpu_flags = [
        '-mcpu=cortex-m7',
        '-mfloat-abi=hard',
        '-mfpu=fpv5-d16',
        '-mthumb'
    ]

    common_defs = [
        'F_CPU=600000000',
        'USB_SERIAL',
        'LAYOUT_US_ENGLISH',
        'USING_MAKEFILE',
        '__IMXRT1062__',
        'ARDUINO=10607',
        'TEENSYDUINO=159',
        'ARDUINO_TEENSY41',
        '__NEWLIB__=1',
        'ALTERNATIVE_MUTEX_IMPL'
    ]

    cflags = cpu_flags + [
        '-std=gnu99',
        '-Wall',
        '-O2',
        '-g',
        '-ffunction-sections',
        '-fdata-sections',
        '--specs=' + nano_specs
    ]

    cxxflags = cpu_flags + [
        '-std=gnu++20',
        '-Wall',
        '-O2',
        '-g',
        '-fno-exceptions',
        '-fno-rtti',
        '-fno-threadsafe-statics',
        '-felide-constructors',
        '-fpermissive',
        '-Wno-error=narrowing',
        '--specs=' + nano_specs
    ]

    includes = [
        SRCPATH,
        COREPATH,
        LIBPATH,
        FREERTOS_CPP_INC
    ]

    # Add all library subdirectories
    for root, dirs, files in os.walk(LIBPATH):
        if 'example' not in root:
            includes.append(root)
            includes.append(os.path.join(root, 'src'))

    # ---------------------------------------------------------------
    # Source discovery
    # ---------------------------------------------------------------
    sources = []

    for path in [SRCPATH, COREPATH, LIBPATH]:
        for ext in ('*.c', '*.cpp'):
            sources += glob.glob(os.path.join(path, '**', ext), recursive=True)

    # Filter out examples
    sources = [s for s in sources if '/example' not in s]

    # ---------------------------------------------------------------
    # ELF target
    # ---------------------------------------------------------------
    elf = bld.program(
        target   = TARGET,
        source   = sources,
        includes = includes,
        defines  = common_defs,
        cflags   = cflags,
        cxxflags = cxxflags,
        linkflags = cpu_flags + [
            '-T' + bld.path.find_resource(bld.env.LDSCRIPT).abspath(),
            '-Wl,--gc-sections,--relax',
            '--specs=' + nano_specs
        ],
        lib = ['m', 'stdc++'],
        features = 'c cxx'
    )

    # ---------------------------------------------------------------
    # Post-build: size + hex
    # ---------------------------------------------------------------
    def post_build(ctx):
        if not hasattr(elf, 'link_task'):
            return

        elf_node = elf.link_task.outputs[0]
        elf_path = elf_node.abspath()
        hex_path = elf_path.replace('.elf', '.hex')

        ctx.exec_command(f"{ctx.env.SIZE} {elf_path}")
        ctx.exec_command(
            f"{ctx.env.OBJCOPY} -O ihex -R .eeprom {elf_path} {hex_path}"
        )


    bld.add_post_fun(post_build)


class reboot(Context.Context):
    cmd = 'reboot'
    fun = 'reboot'

    def execute(self):
        tool = self.path.find_resource('tools/teensy_reboot')
        if not tool:
            self.fatal('tools/teensy_reboot not found')

        self.exec_command(tool.abspath())

class upload(Context.Context):
    cmd = 'upload'
    fun = 'upload'

    def execute(self):
        target = os.path.basename(Context.top_dir)

        # HEX path
        hex_file = os.path.join(
            Context.out_dir,
            target
        )

        if not os.path.isfile(hex_file):
            self.fatal(f'HEX file not found: {hex_file}')

        # Resolve loader path explicitly (NO self.path)
        loader = os.path.join(
            Context.top_dir,
            'tools',
            'teensy_loader_cli'
        )

        if not os.path.isfile(loader):
            self.fatal('tools/teensy_loader_cli not found')

        self.exec_command(
            f'{loader} -w -s -v --mcu=TEENSY41 {hex_file}'
        )