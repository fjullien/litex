#!/usr/bin/env python3

# This file is Copyright (c) 2020 Franck Jullien <franck.jullien@gmail.com>
# License: BSD

from elftools.elf.elffile import ELFFile
import argparse

def print_usage(bios,regions):
    rom_usage = 0
    ram_usage = 0
    
    with open(regions, "r") as regfile:
        for line in regfile:
           if line == 0:
               break
           if 'rom' in line:
               rom_size = int(line.split(' ')[-1], 16)
           if 'sram' in line:
               ram_size = int(line.split(' ')[-1], 16)

    elffile = ELFFile(open(bios, "rb"))
    rom_usage += elffile.get_section_by_name('.text').data_size
    rom_usage = elffile.get_section_by_name('.rodata').data_size
    section = elffile.get_section_by_name('.commands')
    if section:
        rom_usage += section.data_size
    section = elffile.get_section_by_name('.initcalls')
    if section:
        rom_usage += section.data_size
    ram_usage += elffile.get_section_by_name('.data').data_size
    ram_usage += elffile.get_section_by_name('.bss').data_size

    print("\nROM usage: {:.2f}KiB \t({:.2f}%)".format(rom_usage / 1024.0, rom_usage / rom_size * 100.0))
    print("RAM usage: {:.2f}KiB \t({:.2f}%)\n".format(ram_usage / 1024.0, ram_usage / ram_size * 100.0))

def main():
    parser = argparse.ArgumentParser(description="Print bios memory usage")
    parser.add_argument("input", help="input file")
    parser.add_argument("regions", help="region definitions")
    args = parser.parse_args()
    print_usage(args.input, args.regions)


if __name__ == "__main__":
    main()
