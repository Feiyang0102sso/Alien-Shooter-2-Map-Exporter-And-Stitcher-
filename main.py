import os
import sys
import argparse
import logging
import configparser

# 导入我们自己的模块
import stitcher
import compressor

# --- 初始化 ---
if getattr(sys, 'frozen', False):
    application_path = os.path.dirname(sys.executable)
else:
    application_path = os.path.dirname(os.path.abspath(__file__))

try:
    from config import OUTPUT_FOLDER_NAME, LOG_FILE_NAME
except ImportError:
    OUTPUT_FOLDER_NAME = "output"
    LOG_FILE_NAME = "stitching_log.log"


# --- 辅助函数 ---
def load_language(lang_code):
    parser = configparser.ConfigParser()
    lang_file_path = os.path.join(application_path, f'lang_{lang_code}.ini')
    if not os.path.exists(lang_file_path):
        lang_file_path = os.path.join(application_path, 'lang_en.ini')
    parser.read(lang_file_path, encoding='utf-8')
    return parser


def setup_logging(log_path):
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)
    if logger.hasHandlers():
        logger.handlers.clear()
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)
    file_handler = logging.FileHandler(log_path, encoding='utf-8')
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)


# --- 主流程 ---
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("lang", help="Language code (e.g., 'en', 'zh')")
    parser.add_argument("input_dir", help="Input directory for map fragments")
    parser.add_argument("compress_size", nargs='?', default="", help="Target compression size in MB")
    args = parser.parse_args()

    lang_parser = load_language(args.lang)

    if not os.path.isdir(args.input_dir):
        print(lang_parser.get('strings', 'err_path_not_found'))
        return

    # 创建输出目录并配置日志
    output_dir = os.path.join(args.input_dir, OUTPUT_FOLDER_NAME)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    log_file_path = os.path.join(output_dir, LOG_FILE_NAME)
    setup_logging(log_file_path)

    logging.info(lang_parser.get('strings', 'info_starting'))

    # 1. 调用拼接模块
    stitched_image_paths = stitcher.stitch_map(args.input_dir, lang_parser)

    # 2. 如果需要，调用压缩模块
    try:
        compress_mb = float(args.compress_size) if args.compress_size else 0
    except ValueError:
        compress_mb = 0

    if compress_mb > 0 and stitched_image_paths:
        for image_path in stitched_image_paths:
            compressor.compress_image(image_path, compress_mb, lang_parser)

    logging.info(lang_parser.get('strings', 'info_done'))


if __name__ == '__main__':
    main()