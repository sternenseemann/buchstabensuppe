let
  dev-schrift = self: super: {
    libschrift = super.libschrift.overrideAttrs (old: {
      version = "unstable";
      src = self.fetchFromGitHub {
        owner = "tomolt";
        repo = "libschrift";
        rev = "264dec228166fb20669ebe7deb985d123c2d073a";
        hash = "sha256-bztleMa1MAF3EPkQKVP4OfZIM7jrcJVBHpEAa08v0d4=";
      };
    });
  };
in

{ pkgs ? import <nixpkgs> { overlays = [ dev-schrift ]; } }:

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
