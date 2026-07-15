# ChaosCamp26

<details>
<summary>Week 1 - 3D Software Exploration & Fake or Photo Game</summary>

# Task 1 
Downloaded and installed Blender and created a simple scene containing a basic scene. The goal was to get familiar with the software's interface, basic navigation, and object creation/manipulation tools.

### What I did
- Installed Blender
- Explored the viewport navigation (pan, zoom, rotate)
- Created a meshes and applied basic transformations (move, scale, rotate)
- Explored rendering options
## Output
![output](tree.png)


Res: 1080x1080

# Task 2 
Played the Chaos "Fake or Photo" game , which tests the ability to distinguish CGI-rendered images from real photographs. This task helped build an understanding of how photorealistic rendering can be, and what visual cues might (or might not) reveal a rendered image.

### Result
Scored [8/10] correct guesses.Lighting and reflections were the hardest cues to judge.

## Output
![output](week1_task2.png)

</details>
<details>
<summary>Week 2 - PPM Image Generator</summary>

# TASK 1
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
![output](Week2/crt_output_image_preview.png)



# TASK 2

## Description
This program draws a filled ellipse on a 1920x1080 image and writes it to a .ppm file. Pixels inside the ellipse are colored cyan, pixels outside are black.

## Logic
- The ellipse is defined by its center (`center_x`, `center_y`) and semi-axes `x` (horizontal) and `y` (vertical), based on the standard ellipse equation: `(px-h)²/a² + (py-k)²/b² <= 1`.
- For each pixel, its distance from the center is calculated as `double` to avoid integer division errors, which would otherwise produce a jagged/cross shape instead of a smooth curve.
- If the pixel satisfies the equation, it falls inside the ellipse and is colored cyan; otherwise it stays black.

Compile:
```
g++ task2.cpp -o ppm_generator.exe
```
Run:
```
.\ppm_generator.exe
```

The program generates `crt_output_image.ppm` in the same folder.

## Output
![output](Week2/crt_output_image_preview2.png)

</details>

