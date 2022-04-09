# AVX & Alpha-Blending

This is a program that blends 2 pictures into one. It uses AVX 256 registers for parallel computations.

# Installation
run ```make```

# Usage

```$ ./blend.exe [background-image].png [foreground-image].png```

Background image must be bigger or equal to the Screen size, which is hardcoded as 800x600.

# Comparison

| SSE |  Flags | 100 Blending cycles time, s | Speed-up ratio |
|:---:|:------:|:---------------------------:|:--------------:|
|  No | -Ofast |             0.21            |       x1       |
| Yes | -Ofast |             0.06            |      x3.5      |
