# buchstabensuppe

toy font rendering for low pixelcount high contrast displays,
i. e. [openlab's flipdot display](https://wiki.openlab-augsburg.de/Flipdots).

## caveats

* buchstabensuppe loads all fonts into memory and keeps them there pretty much
  all the time.
* buchstabensuppe uses `stb_truetype` which does
  [no boundary checking on fonts](https://github.com/nothings/stb/blob/b42009b3b9d4ca35bc703f5310eedc74f584be58/stb_truetype.h#L4-L11),
  so it is highly inadvisable to use buchstabensuppe with untrusted fonts.
