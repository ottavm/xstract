# xstract: A CLI utility to extract embedded files from Xbox 360 STFS packages.
### Usage:
```
xstract [-p,-f,--package,--file]=PATH [OPTION...]
Options:
    -p, --pkg          : Sets the package file path.
    -f, --file         : Sets the package file path.
    -x, --extract      : Extract the contents of package PATH to OUTPUT.
    -v, --version      : Shows version and copyright notice.
    -h, --help         : Shows this message.
    -o, --output [DIR] : Sets OUTPUT to directory DIR. Default value is './'
```

### Build Instructions
**Warning**: Currently, xstract does not support operating systems other than GNU/Linux. See [^1] 
#### Requirements
1. A C Compiler compatible with the gnu99 standard.
2. CMake >= 3.16 
3. `be16toh`, `be32toh` and `be64toh` functions. (Found in <endian.h> on Linux)
4. `mmap` support. See [^1]

#### Compile commands:
```bash
cd xstract # Source directory

mkdir build
cd build

cmake -S.. -B.
make
```

#### Installation
```bash
sudo make install
```

[^1]: Since I created this tool for my own use, I didn't bother porting it to non-Linux operating systems. Adding support for other Unix-like systems should be relatively straightforward; however, porting it to Windows might be a bit more complicated. This is because the STFS handling library uses `mmap` to load files into the program's address space for the sake of memory efficiency. If you know how (and want) to port the code to your preferred operating system, feel free to contribute. `xstfs.h` assumes a buffer which holds the file contents as an array of bytes.
