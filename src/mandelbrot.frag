#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    vec2 center;
    float zoom;
    float aspectRatio;
    int maxIterations;
} ubo;

// Smooth coloring for Mandelbrot
float mandelbrot(vec2 c, int maxIter) {
    vec2 z = vec2(0.0);
    float iteration = 0.0;

    for (int i = 0; i < 10000; i++) {
        if (i >= maxIter) break;

        float x2 = z.x * z.x;
        float y2 = z.y * z.y;

        if (x2 + y2 > 4.0) {
            // Smooth coloring using normalized iteration count
            float log_zn = log(x2 + y2) * 0.5;
            float nu = log(log_zn / log(2.0)) / log(2.0);
            iteration = float(i) + 1.0 - nu;
            break;
        }

        z = vec2(x2 - y2, 2.0 * z.x * z.y) + c;
        iteration = float(i) + 1.0;
    }

    return iteration;
}

// HSV to RGB conversion for nice colors
vec3 hsv2rgb(vec3 hsv) {
    vec3 rgb = clamp(abs(mod(hsv.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsv.z * mix(vec3(1.0), rgb, hsv.y);
}

// Alternative: palette-based coloring
vec3 palette(float t) {
    // Continuous color palette
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.0, 0.33, 0.67);
    return a + b * cos(6.28318 * (c * t + d));
}

// More sophisticated palette
vec3 mandelbrotPalette(float t) {
    // Multiple color bands
    vec3 color1 = vec3(0.0, 0.0, 0.5);    // Dark blue
    vec3 color2 = vec3(0.0, 0.5, 1.0);    // Cyan
    vec3 color3 = vec3(0.5, 1.0, 0.5);    // Light green
    vec3 color4 = vec3(1.0, 1.0, 0.0);    // Yellow
    vec3 color5 = vec3(1.0, 0.5, 0.0);    // Orange
    vec3 color6 = vec3(1.0, 0.0, 0.0);    // Red
    vec3 color7 = vec3(0.5, 0.0, 0.5);    // Purple
    vec3 color8 = vec3(0.0, 0.0, 0.0);    // Black (interior)

    // Create smooth bands
    float bands = 8.0;
    float band = fract(t * bands);
    float bandIndex = floor(t * bands);

    vec3 colors[8] = {color1, color2, color3, color4, color5, color6, color7, color8};

    // Smooth interpolation between bands
    vec3 c1 = colors[int(bandIndex) % 8];
    vec3 c2 = colors[int(bandIndex + 1.0) % 8];

    // Smoothstep for nicer transitions
    float smooth = smoothstep(0.0, 1.0, band);
    return mix(c1, c2, smooth);
}

// Psychedelic palette
vec3 psychedelicPalette(float t) {
    float r = 0.5 + 0.5 * cos(3.0 + t * 15.0);
    float g = 0.5 + 0.5 * cos(1.0 + t * 15.0);
    float b = 0.5 + 0.5 * cos(0.0 + t * 15.0);
    return vec3(r, g, b);
}

// Fire palette
vec3 firePalette(float t) {
    t = clamp(t, 0.0, 1.0);
    float r = smoothstep(0.0, 0.5, t);
    float g = smoothstep(0.3, 0.8, t);
    float b = smoothstep(0.6, 1.0, t);
    return vec3(r, g, b);
}

// Ocean palette
vec3 oceanPalette(float t) {
    t = clamp(t, 0.0, 1.0);
    vec3 deep = vec3(0.0, 0.1, 0.3);
    vec3 mid = vec3(0.0, 0.4, 0.7);
    vec3 shallow = vec3(0.3, 0.7, 0.9);
    vec3 foam = vec3(0.9, 0.95, 1.0);

    if (t < 0.3) return mix(deep, mid, t / 0.3);
    else if (t < 0.6) return mix(mid, shallow, (t - 0.3) / 0.3);
    else if (t < 0.9) return mix(shallow, foam, (t - 0.6) / 0.3);
    return foam;
}

// Classic Mandelbrot palette (like Fractint)
vec3 classicPalette(float t) {
    // 16-color classic palette simulation
    float idx = t * 16.0;
    int i = int(floor(idx));
    float f = fract(idx);

    vec3 colors[16] = {
        vec3(0.0, 0.0, 0.0),      // 0: Black
        vec3(0.0, 0.0, 0.5),      // 1: Dark blue
        vec3(0.0, 0.0, 1.0),      // 2: Blue
        vec3(0.0, 0.5, 1.0),      // 3: Light blue
        vec3(0.0, 1.0, 1.0),      // 4: Cyan
        vec3(0.5, 1.0, 0.5),      // 5: Light green
        vec3(0.0, 1.0, 0.0),      // 6: Green
        vec3(0.5, 1.0, 0.0),      // 7: Yellow-green
        vec3(1.0, 1.0, 0.0),      // 8: Yellow
        vec3(1.0, 0.75, 0.0),     // 9: Gold
        vec3(1.0, 0.5, 0.0),      // 10: Orange
        vec3(1.0, 0.25, 0.0),     // 11: Red-orange
        vec3(1.0, 0.0, 0.0),      // 12: Red
        vec3(0.75, 0.0, 0.25),    // 13: Magenta
        vec3(0.5, 0.0, 0.5),      // 14: Purple
        vec3(0.25, 0.0, 0.5)      // 15: Dark purple
    };

    vec3 c1 = colors[i % 16];
    vec3 c2 = colors[(i + 1) % 16];
    return mix(c1, c2, smoothstep(0.0, 1.0, f));
}

void main() {
    // Convert UV to complex plane coordinates
    // UV is in [0, 1] x [0, 1], map to complex plane
    vec2 uv = fragUV * 2.0 - 1.0;  // [-1, 1] x [-1, 1]
    uv.x *= ubo.aspectRatio;       // Correct for aspect ratio

    // Apply zoom and center
    vec2 c = ubo.center + uv / ubo.zoom;

    // Compute Mandelbrot
    float iterations = mandelbrot(c, ubo.maxIterations);

    // Normalize iterations
    float t = iterations / float(ubo.maxIterations);

    // Color the point
    vec3 color;

    if (iterations >= float(ubo.maxIterations)) {
        // Interior of Mandelbrot set - black
        color = vec3(0.0);
    } else {
        // Exterior - use palette
        // Try different palettes by changing this line:
        // color = mandelbrotPalette(t * 3.0);        // Multi-band
        // color = psychedelicPalette(t * 5.0);       // Psychedelic
        // color = firePalette(t);                     // Fire
        // color = oceanPalette(t);                    // Ocean
        color = classicPalette(t * 4.0);             // Classic
        // color = palette(t * 10.0);                 // Cosine palette
        // color = hsv2rgb(vec3(t * 0.8, 0.8, 1.0));  // HSV rainbow
    }

    // Add subtle glow effect based on iteration density
    float glow = 1.0 - exp(-iterations * 0.1);
    color *= 0.8 + 0.2 * glow;

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}