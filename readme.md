
## CLI GUID Generator

This small STL-only c++ program prints random or fixed-seeded `GUID` strings
with the format `XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX`, where "X" is an
uppercase hexadecimal digit.

#### Examples

```shell
  # Default single random value
  $ guid-generate
  70D3E1E7-6078-1629-9356-58A4FC10B080

  # Multiple random lines
  $ guid-generate -n4
  95E3DBD6-68AB-08E3-A3BC-3D61FF14A9AA
  8AB7BE86-5931-5A5A-968E-4ABC78431F04
  A2F3C440-26FF-213F-CFED-1775FEC93B7A
  FE1F6EC0-2709-9A26-3648-797CFC708613

  # Text seeded, arguments are implicitly joined
  guid-generate Da drauß vom Walde komm ich her, etc etc.
  D6103D8D-BED1-B817-833B-0022C2F0242B
  # (Same seed, same output)
  guid-generate Da drauß vom Walde komm ich her, etc etc.
  D6103D8D-BED1-B817-833B-0022C2F0242B
```

#### Building the binary

Basically the `main.cc` has to be compiled and linked with the
`c++17` standard or higher.

The Makefile build requires GNU make version >= 4.2 and a compiler/linker with g++ compliant flags. The output binary is statically linked, so that there are no special library dependencies.

- Linux: build-essential package.
- Windows: GNU make, mingw32/64, GIT with its `msys` unix tools in the `%PATH%` (like `cp` or `ls`).
- Mac: gmake, clang++.
