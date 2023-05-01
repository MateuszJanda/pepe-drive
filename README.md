## pepe-drive

Toy project.

### Visual Studio Code and clangd

After generating `compile_commands.json` on linux source code, clangd extension can be used.
```bash
cd ~/linux-kernel
./scripts/clang-tools/gen_compile_commands.py

cp compile_commands.json ~/linux-device-drivers
```

Then reset Visual Studio Code.

### clang-format

```bash
cp ~/linux-kernel/.clang-format ~/linux-device-drivers/

clang-format -style=file:.clang-format -i *.c
clang-format -style=file:.clang-format -i *.h
```

### See also/References

* https://lwn.net/Kernel/LDD3/
