import sys
import json
import psutil
from os import path
from psutil import Process
from time import sleep
from typing import Tuple
import shutil
import subprocess
from subprocess import Popen, PIPE
import re

exit(0)

# ===============
# This script copies the build result into the cmod loader mod dir,
# then runs the intsall script located there.
#
# See the params below -�you WILL need to change atleast some of them,
# if you are planning on building.
# ===============

# absolute path to cosmoteer "bin" directory. escaping separators is not needed.
cosmoteer_bin_dir_path = r"C:\Program Files (x86)\Steam\steamapps\common\Cosmoteer\Bin"

# absolute path to cmod loader mod directory. escaping separators is not needed.
cmod_loader_mod_dir_path = (
    r"C:\Users\aliser\Saved Games\Cosmoteer\76561198068709671\Mods\cmod_loader"
)

# whether to start the cosmoteer process after the build
start_cosmoteer_after_build = True

# whether to start the cosmoteer process in dev mode after the build
# doesn't do anything, if "start_cosmoteer_after_build" is false
start_cosmoteer_in_dev_mode = True

# timeout in ms before restarting the cosmoteer process.
# since Steam takes some time before the process can be restarted, this is required
cosmoteer_process_restart_timeout_seconds = 3

# ===============


def log(*messages: str):
    # encoded_messages: list[str] = []
    # for message in messages:
    # encoded_messages.append(str(message.encode("utf-8")))

    print("[integrate.py | info]", *messages)


# ===============

# init

# path to the current solution, WITH slash at the end
solution_dir = sys.argv[1]
# platform, e.g. x64
platform = sys.argv[2]
# e.g. release/debug
configuration = sys.argv[3]
# path to the cmod loader build output dir
build_dir_path = path.join(solution_dir, platform, configuration)
# path to the cmod loader helper build output dir
helper_build_dir_path = path.join(
    solution_dir, "CMod_LoaderHelper", "bin", configuration, "net7.0-windows"
)
# path to the cmod loader install script (which installs the cmod loader dll into Cosmoteer)
mod_installer_script_file_path = path.join(cmod_loader_mod_dir_path, "install.bat")

log("params:")

log("                            solution dir: ", solution_dir)
log("                                platform: ", platform)
log("                           configuration: ", configuration)
log("                  cosmoteer bin dir path: ", cosmoteer_bin_dir_path)
log("                cmod loader mod dir path: ", cmod_loader_mod_dir_path)
log("              cmod loader build dir path: ", build_dir_path)
log("       cmod loader helper build dir path: ", helper_build_dir_path)
log("  cmod loader installer script file path: ", mod_installer_script_file_path)

log("===============")

# get cosmoteer process
cosmoteer_process: Process | None = None
try:
    cosmoteer_process = [
        p for p in psutil.process_iter() if p.name() == "Cosmoteer.exe"
    ][0]
except IndexError:
    pass

# kill cosmoteer process
if cosmoteer_process is None:
    log("Cosmoteer process is not running, continuing...")
else:
    log("killing Cosmoteer process")
    cosmoteer_process.kill()

    log(
        f"waiting for {cosmoteer_process_restart_timeout_seconds}s before restarting the process"
    )
    sleep(cosmoteer_process_restart_timeout_seconds)

log("===============")

# copy build files to the mod dir

# a preset for files to copy in format: (source, destination)
copy_preset: list[Tuple[str, str]] = [
    (
        path.join(build_dir_path, "CMod_Loader.dll"),
        path.join(
            cmod_loader_mod_dir_path, "bin", "AVRT.dll"
        ),  #                                 ^^^^^^^^ the name of the dll that would be replaced with the loader
    ),
    (
        path.join(build_dir_path, "CMod_Loader.pdb"),
        path.join(
            cmod_loader_mod_dir_path, "bin", "AVRT.pdb"
        ),  #                                 ^^^^^^^^ the name of the dll that would be replaced with the loader
    ),
    #
    (
        path.join(helper_build_dir_path, "CMod_LoaderHelper.dll"),
        path.join(cmod_loader_mod_dir_path, "bin", "CMod_LoaderHelper.dll"),
    ),
    (
        path.join(helper_build_dir_path, "CMod_LoaderHelper.pdb"),
        path.join(cmod_loader_mod_dir_path, "bin", "CMod_LoaderHelper.pdb"),
    ),
    (
        path.join(helper_build_dir_path, "CMod_LoaderHelper.runtimeconfig.json"),
        path.join(
            cmod_loader_mod_dir_path, "bin", "CMod_LoaderHelper.runtimeconfig.json"
        ),
    ),
]

log(f"copying build files to loader mod dir ({len(copy_preset)}):")

for i, (fromPath, toPath) in enumerate(copy_preset):
    log_prefix = f"[{i + 1} of {len(copy_preset)}]"
    log(f"=== {log_prefix}")

    if not path.exists(fromPath):
        raise Exception(f"source path not found: " + fromPath)

    if not path.exists(path.dirname(toPath)):
        raise Exception(f"target dir not found: " + toPath)

    log(f"src : " + fromPath)
    log(f"dist: " + toPath)

    # print separator between logs
    # if i < len(copy_preset) - 1:
    # log("===")

    shutil.copyfile(fromPath, toPath)


log("===============")

log("installing cmod loader onto Cosmoteer")
if not path.exists(mod_installer_script_file_path):
    raise Exception(
        "cmod loader installer script not found: " + mod_installer_script_file_path
    )

# run the installer script with input being redirected from DEVNULL, skipping any pauses
loader_install_script_process = Popen(
    [
        mod_installer_script_file_path,
        "1",  # debug flag
        cosmoteer_bin_dir_path,  # cosmoteer bin dir
    ],
    stdout=PIPE,
)

# get the stdout
loader_install_script_stdout = loader_install_script_process.stdout
if loader_install_script_stdout == None:
    raise Exception("Got None instead of stdout")

# read the output
loader_install_script_output = loader_install_script_stdout.read().decode(
    "ascii", errors="ignore"
)

# file = open("result.txt", "w")
# file.write(loader_install_script_output)
# file.close()

# print the output
for line in re.split(r"\r\n", loader_install_script_output, flags=re.MULTILINE):
    if line == " ":
        # skip empty lines
        continue

    log(f"[{path.basename(mod_installer_script_file_path)}] {line}")


# get the exit code
# loader_install_script_process.communicate()
# process_exit_code = loader_install_script_process.poll()
process_exit_code = loader_install_script_process.wait()

# stop execution if install failed
if process_exit_code != 0:
    # error
    raise Exception("Error while installing the cmod loader (see the output above)")

# close the install script process
loader_install_script_stdout.close()


log("===============")

# start cosmoteer process, if that option is set
if start_cosmoteer_after_build:
    log(
        "starting cosmoteer process"
        + (" in dev mode" if start_cosmoteer_in_dev_mode else "")
    )

    subprocess.run(
        [
            path.join(cosmoteer_bin_dir_path, "Cosmoteer.exe"),
            "--devmode" if start_cosmoteer_in_dev_mode else "",
        ]
    )

    log("===============")


log("all done!")
