#!/bin/bash
#qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 \-machine pc,pcspk-audiodev=audio0 \-audiodev coreaudio,id=audio0

# activa el audio ver esto
