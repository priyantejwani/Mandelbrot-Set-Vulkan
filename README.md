# Mandelbrot Set Visualizer — Vulkan C++

A real-time Mandelbrot set explorer written in **modern C++20** using **raw Vulkan 1.3**. The entire fractal computation runs on the GPU in a fragment shader — no CPU-side iteration loops.

![Demo](docs/demo.gif)

## Features

- **GPU-native Mandelbrot** — every pixel computed in parallel via fragment shader
- **Smooth coloring** — normalized iteration count for continuous gradients (no banding)
- **Multiple palettes** — classic, psychedelic, fire, ocean, cosine/HSV (switch in shader)
- **Interactive navigation** — pan, zoom, iteration control at 60+ FPS
- **Minimal dependencies** — Vulkan SDK, GLFW, GLM only
- **Clean Vulkan codebase** — explicit resource management, validation layers, proper synchronization

## Controls

| Action | Input |
|--------|-------|
| Pan view | Mouse drag / WASD / Arrow keys |
| Zoom in/out | Scroll wheel (at cursor) / **Q** (in) / **E** (out) |
| Increase iterations | **I** |
| Decrease iterations | **K** |
| Reset view | **R** |
| Quit | **Esc** |

Window title displays live FPS, complex-plane center, zoom level, and max iterations.

## Building

### Prerequisites (Windows)

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (1.3+)
- [vcpkg](https://github.com/microsoft/vcpkg) for dependencies

```powershell
# Install dependencies
vcpkg install glfw3 glm vulkan-headers vulkan-loader

# Optional: for development
vcpkg install glfw3:x64-windows glm:x64-windows
```

### Configure & Build

```powershell
cd mandelbrot_vulkan
mkdir build && cd build

# Configure (adjust vcpkg path as needed)
cmake .. `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

### Run

```powershell
./Release/mandelbrot.exe
```

## Project Structure

```
mandelbrot_vulkan/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── LICENSE                     # MIT License
└── src/
    ├── main.cpp                # Entry point, input handling, camera
    ├── vulkan_renderer.hpp     # Vulkan renderer interface
    ├── vulkan_renderer.cpp     # Full Vulkan implementation
    ├── mandelbrot.vert         # Vertex shader (fullscreen quad)
    └── mandelbrot.frag         # Fragment shader (Mandelbrot + palettes)
```

## Customizing Palettes

Edit `src/mandelbrot.frag`, `main()` function (lines ~155–165):

```glsl
// Pick ONE:
color = classicPalette(t * 4.0);      // Classic Fractint-style
// color = mandelbrotPalette(t * 3.0); // Multi-band smooth
// color = psychedelicPalette(t * 5.0); // Rainbow
// color = firePalette(t);              // Black→Red→Yellow→White
// color = oceanPalette(t);             // Deep blue→foam
// color = palette(t * 10.0);           // Cosine wave
// color = hsv2rgb(vec3(t * 0.8, 0.8, 1.0)); // HSV rainbow
```

## How It Works

### Coordinate Mapping
```
Screen UV [0,1]² → NDC [-1,1]² → Complex plane
c = center + (uv * 2.0 - 1.0) * aspect / zoom
```

### Smooth Iteration Count
```glsl
float log_zn = log(x² + y²) * 0.5;
float nu = log(log_zn / log(2.0)) / log(2.0);
smooth_iter = i + 1.0 - nu;  // Continuous, no bands
```

### Vulkan Pipeline
```
Vertex Shader (passthrough)
    ↓
Fragment Shader (Mandelbrot per-pixel)
    ↓
Color attachment → Swapchain → Display
```

Uniform buffer (per-frame) feeds `center`, `zoom`, `aspectRatio`, `maxIterations` to the fragment shader.

## Requirements

| Component | Version |
|-----------|---------|
| C++ Compiler | C++20 (MSVC 19.30+, GCC 11+, Clang 13+) |
| Vulkan SDK | 1.3.200+ |
| CMake | 3.16+ |
| GLFW | 3.3+ |
| GLM | 0.9.9+ |

## License

MIT License — see [LICENSE](LICENSE) for details.

## Acknowledgments

- [Vulkan Tutorial](https://vulkan-tutorial.com/) — excellent learning resource
- [Inigo Quilez](https://www.iquilezles.org/) — palette functions & smooth coloring math
- [Fractint](http://www.fractint.org/) — classic palette inspiration