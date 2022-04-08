# AVX & Alpha-Blending

This is a program that blends 2 pictures into one. It uses AVX 256 registers for parallel computations.

# Installation
run ```make```

# Usage

```$ ./blend.exe [background-image] [foreground-image]```

Background image must be bigger or equal to the Screen size, which is hardcoded as 800x600.
