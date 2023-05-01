#!/usr/bin/env python3

def main():
    with open("/dev/pepe0", "r") as f:
        print("[i] Reading from /dev/pepe0")
        print(f"{f.read()}")


if __name__ == "__main__":
    main()
