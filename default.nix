{ nixpkgsSrc ? <nixpkgs> }:

let
  pkgs = import nixpkgsSrc {
    overlays = [ (import ./overlay.nix) ];
  };
in

pkgs.buchstabensuppe
