## pepe-drive

Toy project. Simple char device with only read access that provides ANSI image. Probably not secure, so use on your own risk.

```bash
$ make
$ sudo ./manage_module.bash load

$ echo -e $(cat /dev/pepe0)
$ sudo dmesg | tail -20


$ sudo ./manage_module.bash unload
```

### Visual Studio Code and clangd

After generating `compile_commands.json` on linux source code, clangd extension can be used.
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
