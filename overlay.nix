self: super:

let
  gi = self.nix-gitignore;

  buchstabensuppe =
    { stdenv, utf8proc, harfbuzz, libschrift, meson, ninja, pkg-config }:
    stdenv.mkDerivation rec {
      pname = "buchstabensuppe";
      version = "unstable";

      src = gi.gitignoreSource [
        "default.nix"
        "overlay.nix"
        "bindings/"   # bindings we don't need for compilation
      ] ./.;

      nativeBuildInputs = [
        meson
        ninja
        pkg-config
      ];

      buildInputs = [ utf8proc harfbuzz libschrift ];

      doCheck = true;
    };

in

{
  buchstabensuppe =
    assert self.lib.versionAtLeast self.libschrift.version "0.10.1";
    self.callPackage buchstabensuppe { };
}
