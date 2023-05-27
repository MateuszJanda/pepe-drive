## pepe-drive

Toy project. A simple Linux driver for in-memory char device. Allow read ANSI image from `/dev/pepe0`. Probably not secure, so use it on your own risk.

```bash
$ make
$ sudo ./manage_module.bash load
$ ls -al /dev/pepe0
  cr--r--r-- 1 root root 509, 0 maj  2 09:51 /dev/pepe0

$ echo -e $(cat /dev/pepe0)
$ sudo dmesg | tail -20

$ sudo ./manage_module.bash unload
```

<p align="center">
<img src="./screenshot.png"/>
</p>

### Visual Studio Code and clangd

After generating `compile_commands.json` on Linux source code, clangd extension can be used.
```bash
$ cd ~/linux-kernel
$ ./scripts/clang-tools/gen_compile_commands.py

$ cp compile_commands.json ~/linux-device-drivers
```

Then reset Visual Studio Code.

### clang-format

```bash
$ cp ~/linux-kernel/.clang-format ~/linux-device-drivers/

$ clang-format -style=file:.clang-format -i *.c
$ clang-format -style=file:.clang-format -i *.h
```

### See also/References/Credits

* https://lwn.net/Kernel/LDD3/
* https://github.com/d0u9/Linux-Device-Driver
* https://dom111.github.io/image-to-ansi/
