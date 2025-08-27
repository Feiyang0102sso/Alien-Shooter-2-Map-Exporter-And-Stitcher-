import os
import io
import logging
from PIL import Image

# Pillow 有一个安全限制，防止解压超大图片（可能是攻击）。我们自己拼接的图很大，需要放宽这个限制。
Image.MAX_IMAGE_PIXELS = None


def compress_image(image_path, target_mb, lang_parser):
    """
    使用二分查找算法，将图片压缩到最接近目标大小的体积。
    """
    if not target_mb or target_mb <= 0:
        return

    logging.info(f"{lang_parser.get('strings', 'info_compressing')} {os.path.basename(image_path)}")

    try:
        with Image.open(image_path) as img:
            if img.mode in ('RGBA', 'P'):
                img = img.convert('RGB')

            # 使用二分查找来寻找最佳质量
            low = 1
            high = 95
            best_quality = -1

            # 我们寻找最高的 quality 使得文件大小 <= target_mb
            while low <= high:
                mid_quality = (low + high) // 2
                if mid_quality == 0: break  # 避免死循环

                buffer = io.BytesIO()
                img.save(buffer, format='JPEG', quality=mid_quality)
                current_size_mb = buffer.tell() / (1024 * 1024)

                if current_size_mb <= target_mb:
                    # 这个质量可用，尝试寻找更高质量
                    best_quality = mid_quality
                    low = mid_quality + 1
                else:
                    # 质量太高，需要降低
                    high = mid_quality - 1

            # 使用找到的最佳质量保存文件
            path_parts = os.path.splitext(image_path)
            compressed_path = f"{path_parts[0]}_compressed.jpg"

            if best_quality != -1:
                final_buffer = io.BytesIO()
                img.save(final_buffer, format='JPEG', quality=best_quality)
                final_size_mb = final_buffer.tell() / (1024 * 1024)
                with open(compressed_path, 'wb') as f:
                    f.write(final_buffer.getvalue())

                # 使用 .format() 来填充模板字符串
                log_msg = lang_parser.get('strings', 'info_compress_success').format(
                    quality=best_quality, size=final_size_mb, path=compressed_path
                )
                logging.info(log_msg)
            else:
                # 如果循环结束 best_quality 仍然是 -1，说明即使在最低质量下也无法满足要求
                img.save(compressed_path, format='JPEG', quality=1)
                log_msg = lang_parser.get('strings', 'info_compress_warning').format(quality=1)
                logging.warning(log_msg)

    except Exception as e:
        log_msg = lang_parser.get('strings', 'info_compress_error').format(error=e)
        logging.error(log_msg)