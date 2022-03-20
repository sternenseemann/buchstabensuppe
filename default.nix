{ pkgs ? import <nixpkgs> { } }:

assert pkgs.lib.versionAtLeast pkgs.libschrift.version "0.10.1";

let
  gi = pkgs.nix-gitignore;

  buchstabensuppe = { stdenv, utf8proc, harfbuzz, libschrift, redo-c }:
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
      buildInputs = [ utf8proc harfbuzz libschrift ];

      doCheck = true;
    };
in

pkgs.callPackage buchstabensuppe { }
