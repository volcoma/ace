#!/usr/bin/env python3
"""
Amalgamate multiple header files into one.
"""
import argparse
from pathlib import Path
import sys


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("root", type=Path,
                        help="Root header file.")

    parser.add_argument("-o", "--output", type=Path, default=None, nargs='?',
                        help="Path to write output to.")

    parser.add_argument("-H", "--header", type=Path, default=None, nargs='?',
                        help="Header to use instead of the one in the root file.")

    parser.add_argument("-e", "--exclude", type=Path, default=[], nargs='*', action='extend',
                        help="Include files to pretend are already included.")

    return parser.parse_args()


seen_includes = set()


def process_file(out, path: Path, drop_header=False):
    in_header = True
    base = path.parent
    out.write("\n")
    with path.open("r") as infile:
        for line in infile.readlines():
            if in_header and drop_header:
                if line.startswith("#"):
                    in_header = False
                else:
                    continue

            if line.startswith("#include"):
                _, inc = line.split()
                incpath = base / inc[1:-1]

                if incpath in seen_includes:
                    continue

                seen_includes.add(incpath)

                if inc.startswith('"'):
                    process_file(out, incpath, drop_header=True)
                    continue

            out.write(line)


def main():
    args = parse_args()

    for e in args.exclude:
        seen_includes.add(e)

    out = sys.stdout if not args.output else args.output.open("w")

    if args.header:
        out.write(args.header.read_text())

    process_file(out, args.root, drop_header=bool(args.header))


if __name__ == '__main__':
    main()
