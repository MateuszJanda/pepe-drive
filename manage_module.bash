#! /bin/bash

module="pepe"
device="pepe"
mode="444"
group=0

function load() {
    echo "Function load()"
    insmod ./$module.ko $* || exit 1

    rm -f /dev/${device}0

    major=$(awk -v device="$device" '$2==device {print $1}' /proc/devices)
    mknod /dev/${device}0 c $major 0

    chgrp $group /dev/${device}0
    chmod $mode /dev/${device}0
    echo "Module ${module}.ko loaded"
}

function unload() {
    echo "Function unload()"
    rm -f /dev/${device}0
    rmmod $module || exit 1
    echo "Module ${module}.ko removed"
}

arg=${1:-""}
case $arg in
    load)
        load ;;
    unload)
        unload ;;
    reload)
        ( unload )
        load
        ;;
    *)
        echo "Remember to run make to compile ${module}.ko first"
        echo "Usage:"
        echo "    $0 {load | unload | reload}"
        exit 1
        ;;
esac
