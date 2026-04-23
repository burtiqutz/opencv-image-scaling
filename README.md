# Image Scaling Library

A C++ image scaling library built with OpenCV that implements zoom (upscaling) and shrink (downscaling) algorithms from scratch, with benchmarking and quality comparison against OpenCV's built-in methods.

## Features

### Zoom (Upscaling)
| Method | Description |
|--------|-------------|
| Nearest Neighbor | Fast pixel replication; blocky at high scale factors |
| Bilinear | Weighted average of 4 neighboring pixels; smooth results |
| Bicubic | Weighted average over a 4×4 pixel grid using a cubic kernel (`a = -0.5`); sharpest results |

### Shrink (Downscaling)
| Method | Description |
|--------|-------------|
| Subsample | Picks one pixel per output cell; fast but aliased |
| Box Average | Averages all source pixels in each output cell; reduces aliasing |

## Project Structure

```
.
├── main.cpp        # Benchmark harness, PSNR comparison, round-trip tests
├── zoom.h          # Zoom function declarations
├── zoom.cpp        # zoomNearestNeighbor, zoomBilinear, zoomBicubic
├── shrink.h        # Shrink function declarations
├── shrink.cpp      # shrinkSubsample, shrinkBoxAverage
└── test.bmp        # Input image (grayscale)
```

## Dependencies

- [OpenCV](https://opencv.org/) (tested with OpenCV 4.x)
- C++17 or later

### Install OpenCV

**Ubuntu/Debian:**
```bash
sudo apt install libopencv-dev
```

**macOS:**
```bash
brew install opencv
```

## Build

```bash
g++ -std=c++17 -O2 main.cpp zoom.cpp shrink.cpp \
    $(pkg-config --cflags --libs opencv4) \
    -o scale
```

## Usage

```bash
./scale <scale_factor> <file.bmp>
```

- `scale_factor > 1.0` — zoom (upscale)
- `scale_factor < 1.0` — shrink (downscale)

### Examples

```bash
./scale 2.0    # zoom 2×
./scale 0.5    # shrink to half size
./scale 1.5    # zoom 1.5×
```

## Output

### Zoom mode (`S > 1`)

Writes six images:

| File | Description |
|------|-------------|
| `out_nearest.bmp` | Custom nearest neighbor |
| `out_bilinear.bmp` | Custom bilinear |
| `out_bicubic.bmp` | Custom bicubic |
| `out_nearest_cv.bmp` | OpenCV `INTER_NEAREST` |
| `out_bilinear_cv.bmp` | OpenCV `INTER_LINEAR` |
| `out_bicubic_cv.bmp` | OpenCV `INTER_CUBIC` |

### Shrink mode (`S < 1`)

Writes four images:

| File | Description |
|------|-------------|
| `out_subsample.bmp` | Custom subsample |
| `out_box.bmp` | Custom box average |
| `out_subsample_cv.bmp` | OpenCV `INTER_NEAREST` |
| `out_box_cv.bmp` | OpenCV `INTER_AREA` |

### Console output

The program prints timing, PSNR vs OpenCV, and round-trip PSNR for each method. Example (zoom 2×):

```
Source: 512x512  S=2

=== ZOOM ===
My  nearest neighbor: 3.21 ms
My  bilinear        : 8.47 ms
My  bicubic         : 24.13 ms
CV  nearest neighbor: 0.41 ms
CV  bilinear        : 0.89 ms
CV  bicubic         : 1.74 ms

=== PSNR vs OpenCV (higher = closer to CV) ===
Nearest:  inf dB
Bilinear: 52.3 dB
Bicubic:  48.7 dB

=== Round-trip PSNR (zoom in then shrink back, higher = better) ===
Nearest:  28.1 dB
Bilinear: 33.6 dB
Bicubic:  34.2 dB
```

## Implementation Notes

### Bicubic kernel

The cubic kernel uses the Keys (1989) formulation with `a = -0.5`:

```
|d| ≤ 1:  (a+2)|d|³ − (a+3)|d|² + 1
1 < |d| ≤ 2:  a|d|³ − 5a|d|² + 8a|d| − 4a
otherwise: 0
```

The 4×4 neighborhood weights are precomputed per output pixel to avoid redundant kernel evaluations in the inner loop.

### Box average shrink

Block size is computed as `floor(1 / S)`. Partial blocks at image boundaries are handled by counting only valid source pixels.

### PSNR metric

Peak Signal-to-Noise Ratio is computed per channel (grayscale) as:

```
PSNR = 10 · log₁₀(255² / MSE)
```

A value of `inf` means pixel-perfect agreement with OpenCV.