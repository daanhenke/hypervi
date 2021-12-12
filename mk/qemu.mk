.PHONY: qemu

OVMF_PATH := ~/Downloads/OVMF.fd

qemu:
	qemu-system-x86_64 --bios $(OVMF_PATH) -drive file=fat:rw:$(DIR_ISO) -m 512m -net none -enable-kvm -cpu host -D qemu.log
