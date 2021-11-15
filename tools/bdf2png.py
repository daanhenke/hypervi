#!/bin/python
from bdfparser import Font
from PIL import Image
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="The path to the bdf")
    parser.add_argument("output", help="The path to the png")
    args = parser.parse_args()

    font = Font(args.input)
    char_width = font.headers['fbbx']

    code_points = [*range(0, 256, 1)]
    img_data = font.drawcps(code_points, mode = 0, linelimit = char_width * 32)
    img = Image.frombytes('1', (img_data.width(), img_data.height()), img_data.tobytes('1'))
    img.save(args.output, 'PNG')

if __name__ == '__main__':
    main()
