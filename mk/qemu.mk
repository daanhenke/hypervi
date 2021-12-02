.PHONY: qemu

OVMF_PATH := ~/Downloads/OVMF.fd

qemu:
	qemu-system-x86_64 --bios $(OVMF_PATH) -drive file=fat:rw:$(DIR_ISO) -net none -d int -enable-kvm -cpu host -smp 8
