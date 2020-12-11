{ pkgs ? import <nixpkgs> { } }:

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

      makeFlags = [ "PREFIX=${placeholder "out"}" ];

      nativeBuildInputs = [ redo-c ];
      buildInputs = [ utf8proc harfbuzz freetype ];

      doCheck = true;
    };
in

pkgs.callPackage buchstabensuppe { }
