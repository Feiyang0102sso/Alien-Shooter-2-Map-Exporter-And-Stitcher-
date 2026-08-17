# AS2 Map Stitcher

将 AS2 地图编辑器导出的 BMP 碎片拼接成 PNG。程序使用 zlib 进行真实压缩，不需要 Pillow 或 Python 环境。

## 输入格式

把同一张地图的碎片放在一个目录中，文件名必须是：

```text
地图名_x_y.bmp
```

例如 `level01_0_0.bmp`、`level01_1_0.bmp`。`x` 从左到右递增，`y` 从上到下递增。不同地图可以放在同一个目录里，程序会分别输出。

当前支持 24 位和 32 位未压缩 BMP，所有碎片尺寸必须一致。

## 源码结构

源码位于 `src/as2_map_stitcher/`：

- `main.cpp`：命令行入口
- `map_stitcher.*`：扫描文件并拼接地图
- `bmp_reader.*`：读取 BMP 碎片
- `png_writer.*`：写出 PNG
- `image.h`：图像数据结构

## 使用

### 双击运行

直接双击 `AS2MapStitcher.exe`，程序会显示菜单：

1. 输入地图碎片所在目录
2. 输入输出目录，直接回车则使用输入目录下的 `output`
3. 等待拼接完成
4. 选择是否删除本次识别到的 BMP 碎片
5. 选择是否继续处理另一个目录

输出的 PNG 文件会集中放在独立的输出目录中。删除选项只会删除符合地图碎片命名规则、并且被本次扫描识别到的 BMP，不会删除其他文件或输出目录。

### 命令行运行

```text
AS2MapStitcher.exe <碎片目录> [输出目录]
```

不指定输出目录时，会在输入目录下创建 `output`，输出文件名为 `地图名_full.png`。

## 构建

需要 CMake 3.20 或更高版本，以及支持 C++17 的编译器：

```text
cmake --preset release
cmake --build --preset release
```

生成的程序位于 `build/release/AS2MapStitcher.exe`。Windows + MinGW 构建时会把所需的 `libwinpthread-1.dll` 自动复制到 exe 旁边，发布时请保留这个 DLL；MSVC 构建启用链接时优化。优先保证构建目录可直接运行，不强求单文件 exe。

PNG 写出会使用 PNG 行过滤和 zlib 压缩，地图中的重复区域可以得到更好的压缩效果。每张 PNG 写出后，命令行会显示 `Created PNG: ...` 日志。
