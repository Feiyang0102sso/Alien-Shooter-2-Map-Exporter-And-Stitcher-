import os
import re
import logging
from collections import defaultdict
from PIL import Image


def stitch_map(input_dir, lang_parser):
    """
    扫描目录，识别并拼接所有地图。返回一个包含所有拼接好的图片路径的列表。
    """
    pattern = re.compile(r'(.+)_(\d+)_(\d+)\.bmp$')
    maps_data = defaultdict(list)
    stitched_files = []

    logging.info(lang_parser.get('strings', 'info_scanning'))
    for filename in os.listdir(input_dir):
        if filename.lower().endswith('.bmp'):
            match = pattern.match(filename)
            if match:
                map_name, x, y = match.groups()
                maps_data[map_name].append({'path': os.path.join(input_dir, filename), 'x': int(x), 'y': int(y)})

    if not maps_data:
        logging.warning(lang_parser.get('strings', 'err_no_files_found'))
        return []

    output_dir = os.path.join(input_dir, "output")  # 假设output文件夹已由主程序创建

    for map_name, tiles in maps_data.items():
        logging.info(f"{lang_parser.get('strings', 'info_stitching')} {map_name}")
        try:
            with Image.open(tiles[0]['path']) as img:
                tile_w, tile_h = img.size
            max_x = max(t['x'] for t in tiles)
            max_y = max(t['y'] for t in tiles)
            full_img = Image.new('RGB', ((max_x + 1) * tile_w, (max_y + 1) * tile_h))

            for tile in tiles:
                with Image.open(tile['path']) as tile_img:
                    full_img.paste(tile_img, (tile['x'] * tile_w, tile['y'] * tile_h))

            output_path = os.path.join(output_dir, f"{map_name}_full.bmp")
            full_img.save(output_path)

            log_msg = f"{lang_parser.get('strings', 'info_stitch_success')} {output_path}"
            logging.info(log_msg)
            stitched_files.append(output_path)

        except Exception as e:
            log_msg = lang_parser.get('strings', 'info_stitch_error').format(map_name=map_name, error=e)
            logging.error(log_msg)

    return stitched_files