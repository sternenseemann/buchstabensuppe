# buchstabensuppe

> at least it's better than PIL.ImageFont

toy font rendering for low pixelcount high contrast displays,
i. e. [openlab's flipdot display](https://wiki.openlab-augsburg.de/Flipdots).

## building

requirements:

* [redo-c](https://github.com/leahneukirchen/redo-c)
* Optionally `make` for install script and such
* [utf8proc](https://juliastrings.github.io/utf8proc)
* [harfbuzz](harfbuzz.github.io/)

```
# run inside nix-shell if you have nix!

# static library
$ redo libbuchstabensuppe.a

# demo binary
$ redo demo.exe

# tests
$ redo test.exe && ./test.exe
```

alternatively you can just run `nix-build`

## demo binary

`./demo.exe` (or `./result/bin/buchstabensuppe-demo` if you use nix)
renders a string using `libbuchstabensuppe` and prints a preview
to `stdout`. it supports usual opentype and truetype fonts, but
won't support e. g. colored fonts.

good results for the rendering target (16px font size, no grayscale)
can be achieved with gnu unifont:

```
$ ./demo.exe "Hello 😸" ./fonts/*.ttf 2> /dev/null
Added font ./fonts/unifont.ttf
Added font ./fonts/unifont_upper.ttf
Font count: 2
                                                               
                                                               
                                                               
                                                               
                                                   █       █   
                   ██      ██                     █ █ ███ █ █  
 █    █             █       █                     █  █   █  █  
 █    █             █       █                    █           █ 
 █    █   ████      █       █     ████           █   █   █   █ 
 █    █  █    █     █       █    █    █          █  █ █ █ █  █ 
 ██████  █    █     █       █    █    █         ██           ██
 █    █  ██████     █       █    █    █          ██ ███████ ██ 
 █    █  █          █       █    █    █         ██  █ █ █ █  ██
 █    █  █          █       █    █    █           █  █████  █  
 █    █  █    █     █       █    █    █            ██     ██   
 █    █   ████    █████   █████   ████               █████     
Dimensions: (63, 16)
```

## caveats

* buchstabensuppe loads all fonts into memory and keeps them there pretty much
  all the time.
* buchstabensuppe uses `stb_truetype` which does
  [no boundary checking on fonts](https://github.com/nothings/stb/blob/b42009b3b9d4ca35bc703f5310eedc74f584be58/stb_truetype.h#L4-L11),
  so it is highly inadvisable to use buchstabensuppe with untrusted fonts.
