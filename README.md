# zx_conversion_utils

Utilities for reading ZX Disc images and file conversion utilities.

This is a collection of quickly hacked together utilities to extract files from various disc formats.

## PlusD -> TAP converter

An interactive tool to extract files from +D format discs to .TAP files.

## Plus3 Read

Having failed to read my dumped discs with libdsk/cpmtools, cpcfs and cpcxfs it was
time to hack up a quick tool to extract what I can from disc images.

The images I was working on were 720k B: drive images using out and back formatting. 

## DeOCP

I originally wrote all of my Z80 code using a modified version of the OCP Editor/Assembler. This
used a minimally compressed file format with embedded line numbers. This program generates a standard text file from an input file. The text file can then of course be assembled using a
cross-assembler after the pseudo opcodes are cleaned up.


