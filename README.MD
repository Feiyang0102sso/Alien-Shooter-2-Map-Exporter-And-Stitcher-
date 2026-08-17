# AS2 Map Stitcher

## 简体中文

将 AS2 地图编辑器导出的 BMP 碎片拼接成 PNG 图片。

### 使用方法

1. 将地图碎片放在同一个文件夹中。
2. 确保文件名格式为：`地图名_x_y.bmp`。
   例如：`level01_0_0.bmp`、`level01_1_0.bmp`。
3. 双击 `AS2MapStitcher.exe`。
4. 输入碎片所在文件夹路径。
5. 输入输出文件夹路径；直接回车会使用输入目录下的 `output` 文件夹。
6. 程序完成后会询问是否删除 BMP 碎片，以及是否继续处理其他文件夹。

生成的 PNG 会保存在输出文件夹中。

发布时请将以下两个文件放在同一个目录：

```text
AS2MapStitcher.exe
libwinpthread-1.dll
```

### 命令行用法

```text
AS2MapStitcher.exe <碎片目录> [输出目录]
```

---

## English

This tool stitches BMP map fragments exported by the AS2 map editor into PNG images.

### How to use

1. Put all map fragments in the same folder.
2. Make sure the filenames follow this format: `mapname_x_y.bmp`.
   For example: `level01_0_0.bmp`, `level01_1_0.bmp`.
3. Double-click `AS2MapStitcher.exe`.
4. Enter the folder containing the fragments.
5. Enter an output folder. Press Enter to use the `output` folder inside the input directory.
6. After stitching, the program asks whether to delete the BMP fragments and whether to process another folder.

The generated PNG files are saved in the output folder.

When distributing the program, keep these two files in the same folder:

```text
AS2MapStitcher.exe
libwinpthread-1.dll
```

### Command-line usage

```text
AS2MapStitcher.exe <input-folder> [output-folder]
```
