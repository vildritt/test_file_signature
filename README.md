# Description

CLI file segmented signature tool

see src/usage.txt also

for given input file IF (size <= 128Gb), block size S (512b..10Mb), output file OF:
    - split file to blocks if size S (fill last one with zeros if it's not fit)
    - evaluate MD5 hash for each block
    - put block hashes sequentaly to output file OF line by line

Restictions:
    - C++17 is allowed
    - do not use external libs
    - do not use memory mapped files
    - optimize for performance

# For developer

MD5 code used from (BSD):
    https://openwall.info/wiki/people/solar/software/public-domain-source-code/md5

## Dependencies

- C++17
- CMake >= 3.10
- little-endian processors (not checked on BE!)

## Build

Common cmake way
