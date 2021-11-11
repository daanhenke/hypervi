#!/bin/python

from PIL import Image
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="The path to the png")
    parser.add_argument("output", help="The path to the header")
    args = parser.parse_args()

    if args.output == "image":
        output = convertBinToImage(args)
        output.save(args.image)
    elif args.output == "binary":
        output = convertImageToBinary(args)
        with open(args.binary,"wb+") as f:
            f.write(output)

    else:
        print("Unknown arg for output: ",args.output)


if __name__ == '__main__':
    main()
