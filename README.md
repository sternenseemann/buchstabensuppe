# buchstabensuppe

```
 █             
 █             
 █             
 █ ███    ████ 
 ██   █  █    █
 █    █  █     
 █    █   ██   
 █    █     ██ 
 █    █       █
 ██   █  █    █
 █ ███    ████ 
```

> at least it's better than PIL.ImageFont

toy font rendering for low pixelcount high contrast displays,
i. e. [openlab's flipdot display](https://wiki.openlab-augsburg.de/Flipdots).

## features

* supports [text shaping](https://harfbuzz.github.io/what-is-harfbuzz.html#what-is-text-shaping)
  via harfbuzz
* per grapheme cluster font fallback
* grayscale and binary b/w support

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
$ redo bs-renderflipdot.exe

# tests
$ redo test.exe && ./test.exe
```

alternatively you can just run `nix-build`

## demo

if you want to play around with the font rendering in binary
mode you can use the dry run mode of the supplied `./bs-renderflipdot.exe`
(or `./result/bin/bs-renderflipdot` if you use nix) binary.

Run `./bs-renderflipdot.exe -?` for usage instructions and don't forget `-n`
for dry running!

```
$ ./bs-renderflipdot.exe -f fonts/unifont.ttf -f fonts/unifont_upper.ttf -n "Greetings ❣️" 2>/dev/null
                                                                                            
                                                                                    ███ ███ 
                                                                                   █████████
                                            █                                      █████████
  ████                             █        █                                      █████████
 █    █                            █                          █                     ███████ 
 █    █  █ ███    ████    ████     █       ██    █ ███    ███ █   ████                ███   
 █       ██   █  █    █  █    █  █████      █    ██   █  █   █   █    █                █    
 █       █    █  █    █  █    █    █        █    █    █  █   █   █                          
 █  ███  █       ██████  ██████    █        █    █    █  █   █    ██                  ███   
 █    █  █       █       █         █        █    █    █   ███       ██               █████  
 █    █  █       █       █         █        █    █    █   █           █              █████  
 █   ██  █       █    █  █    █    █        █    █    █   ████   █    █              █████  
  ███ █  █        ████    ████      ██    █████  █    █  █    █   ████                ███   
                                                         █    █                             
                                                          ████                              
```

## caveats

* buchstabensuppe loads all fonts into memory and keeps them there pretty much
  all the time.
* buchstabensuppe uses `stb_truetype` which does
  [no boundary checking on fonts](https://github.com/nothings/stb/blob/b42009b3b9d4ca35bc703f5310eedc74f584be58/stb_truetype.h#L4-L11),
  so it is highly inadvisable to use buchstabensuppe with untrusted fonts.
