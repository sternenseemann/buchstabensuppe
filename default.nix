{ nixpkgsSrc ? <nixpkgs> }:

let
  pkgs = import nixpkgsSrc {
    overlays = [ (import ./overlay.nix) ];
  };
in

pkgs.buchstabensuppe.overrideAttrs (old: {
  passthru = old.passthru or {} // {
    inherit (pkgs.haskellPackages) haskell-buchstabensuppe;
  };
})
