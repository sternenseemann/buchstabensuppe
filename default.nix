{ pkgs ? import /home/lukas/src/nix/nixpkgs {} }:

let
  gi = pkgs.nix-gitignore;
  buchstabensuppe = { stdenv, utf8proc, harfbuzz, freetype, redo-c }:
    stdenv.mkDerivation rec {
      pname = "buchstabensuppe";
      version = "unstable";

      src = builtins.path {
        path = ./.;
        name = "buchstabensuppe-src";
        filter = gi.gitignoreFilter (builtins.readFile ./.gitignore) ./.;
      };

      nativeBuildInputs = [ redo-c ];
      buildInputs = [ utf8proc harfbuzz freetype ];

      buildPhase = "redo demo.exe";

      installPhase = ''
        install -Dm755 demo.exe $out/bin/buchstabensuppe-demo
      '';

      doCheck = true;
      checkPhase = ''
        redo test.exe
        ./test.exe
      '';
    };
in

pkgs.callPackage buchstabensuppe { }
