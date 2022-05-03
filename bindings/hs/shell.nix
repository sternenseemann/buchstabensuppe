{ nixpkgsSrc ? <nixpkgs> }:

let
  pkgs = import nixpkgsSrc {
    overlays = [ (import ../../overlay.nix) ];
  };
in

pkgs.haskellPackages.shellFor {
  packages = p: [
    p.haskell-buchstabensuppe
  ];

  nativeBuildInputs = [
    pkgs.cabal-install
  ];

  withHoogle = true;
}
