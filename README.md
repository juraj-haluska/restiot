# restIoT

restIoT is simple web server (HTTPS only) running on embedded devices which are supported by mbed OS from ARM (Nucleo-F767ZI was used for development). The primary goal of this software is to expose board's functionality by REST API.

## How to run it


### mbed CLI

This project is managed by mbed CLI. I recomend you to use it too. Instructions on mbed CLI installation
can be found in its [repository](https://github.com/ARMmbed/mbed-cli#installing-mbed-cli).

### Import this project by mbed CLI
```
mbed import https://github.com/spacive/restiot
```

### ARM GCC

1. Download and unpack ARM GCC compiler to your favorite directory. You can download it [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm0). If you want, you can use other compilers supported by mbed OS, however, ARM GCC was tested only.

2. Set GCC ARM compiler path in restiot/mbed_settings.py

### Generate certificate and key
1. Change to restiot/cert directory
```
cd restiot/cert
````
2. Generate certificate and key by running certgen.sh
```
./certgen.sh
````
This script generates 2048-bit certificate and private key using openssl. Certificate and key are stored in CERT.h, KEY.h files and they will be appended to binary image by compilation.

### Compile using mbed CLI
```
mbed compile -m [your_board] -t GCC_ARM
```

### Load image to the board
1. Plug your board into USB
2. Copy compiled file (restiot.bin somewhere in BUILD directory) to your board, or copy it by following script (invoke in restiot dir):
```
 sudo ./load /dev/sd[X] BUILD/[your_board]/GCC_ARM/restiot.bin