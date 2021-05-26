# An interpreter and renderer for RUIDA rd-files

## Usage
```
Usage: rdint [options] <RD-file> [<output file>]

Options:
  -i                Enter interactive mode
  -a                Automatically crop the output image to the detected bounding box
  -c <bbox>         Clip to given bounding box
  -v <filename>     Output the vector pass to the given filename
  -r <filename>     Output the raster pass to the given filename
  -d <level>        Set the verbosity level (quiet/info/warn/debug)
  -s <dimension>    Configure the size of the live rendering window. e.g. 1024x768
```

## Dependencies
X11, SDL, SDL_gfx

## Build
```
make -j8
```

## Install
```
sudo make install
```


