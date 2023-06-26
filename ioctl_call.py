import os
import fcntl

device_file = "/dev/pepe0"
fd = os.open(device_file, os.O_RDONLY)

ioctl_cmd = 0x80046b00  # PEPE_IOCTL_CMD_IS_WEDNESDAY
ioctl_arg = bytearray(4)

try:
    retval = fcntl.ioctl(fd, ioctl_cmd, ioctl_arg)
    print(f"Call ioctl() PEPE_IOCTL_CMD_IS_WEDNESDAY for {device_file}:")
    print(f"It's Wednesday?: {retval}")
except Exception as e:
    print(f"ioctl error: {e}")

os.close(fd)
