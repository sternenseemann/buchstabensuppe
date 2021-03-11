{ depotSrc ? builtins.fetchGit {
    url = "https://code.tvl.fyi";
    ref = "canon";
    rev = "2cd2b58a04cd86e8bf1d72e9c0a67ad8c8e9c8dd";
  }
}:

let
  depot = import depotSrc { };
in

depot.users.sterni.htmlman {
  title = "buchstabensuppe";
  pages = [
    {
      name = "bs-renderflipdot";
      section = 1;
    }
  ];
  manDir = ./man;
  description = ''
    * [Source (GitHub)](https://github.com/sternenseemann/buchstabensuppe)
    * [Source (Mirror)](https://code.sterni.lv/buchstabensuppe/)

    buchstabensuppe is a simple font rendering library intended
    for rendering fonts on binary black and white displays
    with low resolutions, for example
    [OpenLab's flipdot display](https://wiki.openlab-augsburg.de/Flipdots).
    The goal is to implement font rendering that
    is adequate for the situation and as correct as possible
    (e. g. does text shaping, …).

    Documentation is still a bit sparse, but shall be improved.
    For now most information is in the
    [README](https://github.com/sternenseemann/buchstabensuppe/blob/main/README.md).
  '';
}
