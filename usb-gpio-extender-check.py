#!/usr/bin/env python3

import argparse
import logging
import os
import subprocess
import sys
import time


PIN_COUPLE_COUNT = 5

logger = logging.getLogger(__name__)


class DUT:
    STTY_ARGS = (
        "ospeed 115200 ispeed 115200 time 5 "
        "-parenb -parodd -cmspar cs8 -hupcl -cstopb cread clocal -crtscts "
        "ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff "
        "-opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 "
        "-isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt "
        "-echoctl -echoke -flusho -extproc"
    )

    def __init__(self, dut):
        self.dut = dut

    def run(self, cmd, verbose=False):
        retval = ""

        subprocess.run(f"stty -F {self.dut} {self.STTY_ARGS}", shell=True, check=True)

        fd = os.open(self.dut, os.O_RDWR|os.O_NOCTTY)
        try:
            if verbose:
                print(cmd)
            os.write(fd, cmd.encode())
            retval = os.read(fd, 128).decode().strip()
        finally:
            os.close(fd)

        return retval


def test_wrapper(test, dut, testname, description):
    start_test_head = "=" * 40
    end_test_head = "-" * 40
    print(f"{start_test_head}\nTest name: {testname}\nDescription: {description}\n{end_test_head}")
    try:
        retval = test(dut)
    except Exception:
        logger.exception("Execution error")
        return 1
    print(f"Test name: {testname} - {'FAILED' if retval else 'PASSED'}")
    return retval


def test_I_command(dut):
    info = dut.run("~I")
    print(info)
    return 0 if len(info) > 0 else 1


def test_P_A_commands(dut):
    delay = 0.1
    err_count = 0

    for i in range(2 ** PIN_COUPLE_COUNT):
        state = bin(i)[2:].zfill(PIN_COUPLE_COUNT)
        print(f"Set state {state}")
        ret_P = dut.run(f"~P{state}")
        time.sleep(delay)

        ret_A = dut.run(f"~A")
        time.sleep(delay)
        if ret_A[2:] != ret_P[2:]:
            print(f"Error on {state} case")
            err_count += 1
    return 1 if err_count else 0


def test_S_R_G_commands(dut):
    err_count = 0
    delay = 0.1

    def loop_body(cmd):
        ret = dut.run(f"~{cmd}{pin}", verbose=True)
        time.sleep(delay)
        ret_G = dut.run(f"~G{pin + PIN_COUPLE_COUNT}")
        time.sleep(delay)
        digit = "1" if cmd == "S" else "0"
        result = ret_G[-1:] == digit
        if not result:
            print(
                (
                f"Error on output pin {pin} and input pin {pin + PIN_COUPLE_COUNT}. Command "
                f"'~{cmd}{pin}' return {ret}. Command '~G{pin + PIN_COUPLE_COUNT}' return {ret_G}"
                )
            )
        return 0 if result else 1

    for pin in range(1, PIN_COUPLE_COUNT + 1):
        err_count += loop_body("S")
        err_count += loop_body("R")

    return 1 if err_count else 0


def test_B_command(dut):
    dut.run("~P11111", verbose=True)
    assert dut.run("~A") != "~A00000"
    dut.run("~B")
    time.sleep(1)
    return 0 if dut.run("~A") == "~A00000" else 1


def main():
    parser = argparse.ArgumentParser(description="My example explanation")
    parser.add_argument(
        "-d",
        "--dut",
        required=True,
        help="USB GPIO Extender device (/dev/ttyACM*)",
    )
    args = parser.parse_args()

    dut = DUT(args.dut)

    err = 0
    # TODO: Add testcases with wrong values
    err += test_wrapper(test_I_command, dut, "test_I_command", "Command I test - print information")
    err += test_wrapper(test_P_A_commands, dut, "test_P_A_commands", "Commands P and A test - multiple set and get")
    err += test_wrapper(test_S_R_G_commands, dut, "test_S_R_G_commands", "Commands S, R and G test - set/reset one pin")
    err += test_wrapper(test_B_command, dut, "test_S_R_G_commands", "Commands S, R and G test - set/reset one pin")
    return err


if __name__ == "__main__":
    sys.exit(main())
