import sys
from pathlib import Path
from PIL import Image

def convert_to_rgb565(filename):
    base = Path(filename).stem
    var_name = base
    output_file = f"{base}.h"

    img = Image.open(filename).convert('RGB')
    width, height = img.size

    pixels = list(img.getdata())  # <-- FIXED
    expected_len = width * height
    if len(pixels) != expected_len:
        raise ValueError(
            f"? Pixel data length mismatch! Expected {expected_len}, got {len(pixels)}"
        )

    with open(output_file, "w") as f:
        f.write("#include <pgmspace.h>\n\n")
        f.write(f"// {var_name}, {width}x{height}px\n")
        f.write(f"const uint16_t {var_name}[{width * height}] PROGMEM = {{\n")

        for i, (r, g, b) in enumerate(pixels):
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            f.write(f"0x{rgb565:04X}, ")
            if (i + 1) % 12 == 0:
                f.write("\n")

        f.write("};\n")

    print(f"? Converted {width}x{height} image ? {output_file}")
    print(f"   Variable name: {var_name}")
    print(f"   Array length: {width * height}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python convert.py <imagefile>")
    else:
        convert_to_rgb565(sys.argv[1])

# Example internal call:
convert_to_rgb565("FS_Fireworks.jpg")

# To call if from a cmd window:
# python convertJPG.py FireworksPunchThrough1.jpg
