{ depotSrc ? builtins.fetchGit {
    url = "https://code.tvl.fyi";
    ref = "canon";
    rev = "0e3858b5e5bb5717319c5a17fb195c51f1cbf06c";
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
  linkXr = "none"; # TODO(sterni): add the remaining man pages
}
