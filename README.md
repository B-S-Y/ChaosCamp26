# ChaosCamp26


<details>
<summary>Week 2 - PPM Image Generator</summary>

This program generates a 1920x1080 image and writes it to a .ppm file. The image is divided into a 4x4 grid (16 rectangles), and each rectangle is assigned one of 6 color groups: red, green, yellow, blue, magenta, cyan.

## Logic
- The image is split into `gridCols x gridRows` (4x4) sections, with each section having pixel dimensions of `rectWidth x rectHeight` (480x270).
- For each pixel, its cell position (row, col) is determined using `rowIdx / rectHeight` and `colIdx / rectWidth`.
- The cell position is converted into a single sequential index using `cellIndex = row * gridCols + col` (left to right, top to bottom, row by row).
- A color group is assigned to each cell using `cellIndex % 6`. This formula guarantees that horizontally and vertically adjacent cells always fall into different color groups.
- Within each color group, the relevant color channel(s) get a random value between 0-255 per pixel, while the remaining channel(s) stay fixed at 0.

## How to run
Compile:
```
g++ task1.cpp -o ppm_generator.exe
```
Run:
```
.\ppm_generator.exe
```

The program generates a file named `crt_output_image.ppm` in the same folder. You can view it using an image viewer such as GIMP, Krita...

## Output
![output](week2/crt_output_image_preview.png)

</details>

