#!/bin/bash

# Intentar ejecutar con audio primero
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 \
    -machine pc,pcspk-audiodev=audio0 \
    -audiodev coreaudio,id=audio0 2>/dev/null

# Si falla (exit code != 0), ejecutar sin audio
if [ $? -ne 0 ]; then
    echo "Audio no disponible, ejecutando sin audio..."
    qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512
fi
