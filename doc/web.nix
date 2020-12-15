{ pkgs ? (import <nixpkgs> {}) }:

let
  manPages = [
    { name = "bs-renderflipdot"; section = 1; }
  ];

  readmeDeps = [
    "bs-renderflipdot.c"
    "doc/flipdot-render.png"
  ];

  lib = pkgs.lib;
  repoRoot = ./..;

  # TODO: man inclusion -Oman
  buildManPage = { name, section }: ''
    ${pkgs.mandoc}/bin/mandoc -Thtml -Ostyle=../style.css \
      ${repoRoot}/doc/man/${name}.${toString section} \
      > $out/man/${name}.${toString section}.html
  '';

  readme = pkgs.runCommandLocal "README.html" {} ''
    echo ${lib.escapeShellArg (commonHead 1)} > $out
    echo "<body><main>" >> $out
    ${pkgs.lowdown}/bin/lowdown ${repoRoot}/README.md >> $out
    echo "</main></body></html>" >> $out
  '';

  normalize = pkgs.fetchurl {
    url = "https://necolas.github.io/normalize.css/8.0.1/normalize.css";
    sha256 = "04jmvybwh2ks4dlnfa70sb3a3z3ig4cv0ya9rizjvm140xq1h22q";
  };

  style = pkgs.writeText "style.css" ''
    body {
      font-size: 1em;
      font-family: serif;
      background-color: #efefef;
      line-height: 1.5;
    }

    h1, h2, h3, h4, h5, h6 {
      font-family: 'Source Sans Pro', 'Open Sans', sans-serif;
    }

    a:link, a:visited {
      color: #3e7eff;
    }

    header, main, footer, table.head, table.foot, div.manual-text {
      margin-top: 0;
      margin-bottom: 0;
      margin-left: auto;
      margin-right: auto;
      max-width: 800px;
      background-color: white;
      padding: 5px 10px;
    }

    /* markdown */

    pre {
      background-color: #efefef;
      overflow: auto;
      padding: 10px;
    }

    img {
      max-width: 100%;
    }

    /* man page related */
    table.head, table.foot {
      width: 100%;
    }

    table.head {
      padding-bottom: 20px;
    }

    table.foot {
      padding-top: 20px;
    }

    table.head td, table.foot td {
      text-align: center;
      padding: 0;
    }

    table.head td:nth-last-child(1),
    table.foot td:nth-last-child(1) {
      text-align: right;
    }

    table.head td:nth-child(1),
    table.foot td:nth-child(1) {
      text-align: left;
    }
  '';

  commonHead = depth:
    let
      styleDir = lib.concatStrings (builtins.genList (x: "../") depth);
    in ''
    <!doctype html>
    <html lang="en">
      <head>
        <meta charset="utf-8">
        <title>buchstabensuppe</title>
        <link rel="stylesheet" type="text/css" href="${styleDir}style.css">
      </head>
  '';

  index = pkgs.writeText "index.html" ''
      ${commonHead 0}
      <body>
        <header>
          <h1>buchstabensuppe</h1>
          <nav>
            <ul>
              <li>
                <a href="https://code.sterni.lv/buchstabensuppe/">
                  Source (Mirror)
                </a>
              </li>
              <li>
                <a href="https://github.com/sternenseemann/buchstabensuppe">
                  Source (Github)
                </a>
              </li>
            </ul>
          </nav>
        </header>
        <main>
          <section>
            <h2>description</h2>
            <p>
              buchstabensuppe is a simple font rendering library intended
              for rendering fonts for binary black and white displays
              with low resolutions, for example
              <a href="https://wiki.openlab-augsburg.de/Flipdots">
                OpenLab's flipdot display
              </a>. The goal is to implement font rendering that
              is adequate for the situation and as correct as possible
              (e. g. do text shaping, …).
            </p>
            <p>
              Documentation is still a bit sparse, but shall be improved.
              For now most information is in the
              <a href="readme/README.html">README</a>.
            </p>
          </section>
          </section>
          <section>
            <h2>man pages</h2>

            <ul>
              ${lib.concatMapStrings ({ name, section }: ''
                  <li>
                    <a href="man/${name}.${toString section}.html">
                      ${name}(${toString section})
                    </a>
                  </li>
                '') manPages}
            </ul>
          </section>
        </main>
      </body>
    </html>
  '';

in

pkgs.runCommandLocal "buchstabensuppe-web" {} ''
  mkdir -p $out/man
  mkdir -p $out/readme
  cat ${normalize} ${style} > $out/style.css
  cp ${index} $out/index.html

  ${lib.concatMapStrings buildManPage manPages}

  cp ${readme} $out/readme/README.html

  ${lib.concatMapStrings (dep: ''
      mkdir -p "$(dirname "$out/readme/${dep}")"
      cp "${repoRoot}/${dep}" "$out/readme/${dep}"
    '') readmeDeps}
''
