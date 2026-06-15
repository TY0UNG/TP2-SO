
#Es para pasarlo en computadora fisica 
# despyes toca pasar el So_arrancable.img a la usb 
#sudo dd if=mi_os_arrancable.img of=/dev/<el disco /usb > bs=4m status=progress

qemu-img convert -f qcow2 -O raw Image/x64BareBonesImage.qcow2 So_arrancable.img