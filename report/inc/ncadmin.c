ACTION=="add", SUBSYSTEM=="netchar", MODE="0666"

$ udevadm info -qall -n /dev/input/event8
P: /devices/virtual/input/input8/event8
N: input/event8
E: DEVNAME=/dev/input/event8
E: DEVPATH=/devices/virtual/input/input8/event8
E: ID_INPUT=1
E: ID_INPUT_KEY=1
E: SUBSYSTEM=input
