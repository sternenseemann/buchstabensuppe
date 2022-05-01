self: super:

let
  gi = self.nix-gitignore;

  buchstabensuppe = { stdenv, utf8proc, harfbuzz, libschrift, redo-c }:
    stdenv.mkDerivation rec {
      pname = "buchstabensuppe";
      version = "unstable";

      src = gi.gitignoreSource [
        "default.nix"
        "overlay.nix"
        "bindings/"   # bindings we don't need for compilation
      ] ./.;

      makeFlags = [ "PREFIX=${placeholder "out"}" ];

      nativeBuildInputs = [ redo-c ];
      buildInputs = [ utf8proc harfbuzz libschrift ];

      doCheck = true;
    };

in

{
  buchstabensuppe =
    assert self.lib.versionAtLeast self.libschrift.version "0.10.1";
    self.callPackage buchstabensuppe { };
}
