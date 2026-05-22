# How It Works

**cleanimg** operates in two distinct modes:

### 1. Erase Mode (`erase`)
Destructively overwrites the original file **in-place**.
* **How it works:** It strips all EXIF metadata tags directly from the file on disk without loading the entire image into memory.

### 2. Remove Mode (`remove`)
Safely creates a clean copy **without altering the original file**.
* **How it works:** It loads the image into an internal representation, strips the EXIF metadata, and saves it as a new, clean image.

# Building
## Requirements
- A POSIX compliant Operating System with ```mmap()``` support.
- A C Compiler. (C11 or later)

- Clone this repository or [download the latest release](https://github.com/ottavm/cleamimg/releases/latest) (recommended)

### From Latest Release (Recommended)

1. [Download the latest release](https://github.com/ottavm/cleanimg/releases/latest) and unpack the tarball.
2. Build the project:

```bash
mkdir build && cd build   # Optional out-of-tree build
../configure              # Use ./configure if you skipped the build directory
make

```

*The compiled executable will be available at src/cleanimg (or build/src/cleanimg).*

### From Source (Clone)

1. Clone and prepare the repository:

```bash
git clone https://github.com/ottavm/cleanimg.git
cd cleanimg
autoreconf -fi

```

2. Build the project:

```bash
mkdir build && cd build   # Optional out-of-tree build
../configure              # Use ./configure if you skipped the build directory
make

```

### Installing / Uninstalling

Run these commands from inside your build directory:

```bash
sudo make install
# sudo make uninstall # To uninstall

```

Feel free to contribute!
