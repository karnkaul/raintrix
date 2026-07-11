# raintrix

**Matrix-inspired text rain effect**

https://github.com/user-attachments/assets/3ee7997a-15fe-4185-9704-211b94b366d8

[![Build Status](https://github.com/karnkaul/raintrix/actions/workflows/ci.yml/badge.svg)](https://github.com/karnkaul/raintrix/actions/workflows/ci.yml)

## Requirements

- Vulkan 1.3+ loader, driver, and capable GPU
- Desktop compositor (Wayland/X11/Win32)
- (Windows) Latest C++ Runtime
- (Linux) libc, libm, libgcc

## Usage

Download the [latest release](https://github.com/karnkaul/raintrix/releases) or build from source, and run the executable. Pass `--generate` to print generated (default) configuration to standard output.

## Features

All supported features are exposed through the configuration file. Generated config has annotated fields.

- Customizable font
- Customizable character set to sample glyphs from (ASCII only)
- Fullscreen or windowed
- Customizable tint, max trails, density, tile height, max depth, and speed
- Keybind to show statistics (and tweak density / vsync)

## Attribution

- [Sketch font](https://www.fontspace.com/sketch-font-f26335) (embedded into binary)

## Contributing

Pull/merge requests are welcome.

**[Original repository](https://github.com/karnkaul/raintrix)**
