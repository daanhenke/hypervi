# violet
a work in progress l1 hypervisor

## Dependencies
submodules are used for ia32doc, to initialize them call
```
git submodule init
git submodule update ---init
```

you need the following programs installed on a unix system
```
make
mkfs.msdos
clang
lld-link
nasm
python3
```

to install the python dependencies run the following command
```
pip install -r ./tools/requirements.txt
```

## Building
to build the hypervisor and loader run
```
make all
```
this will put outputs in `build/debug/dist`

there's some other make commands you can use in combination with all to make your life a bit easier
```sh
make all qemu # starts a qemu instance
make all iso # generates build/debug/violet.iso
```
