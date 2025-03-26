{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils, ... }: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };
    in
    {
      devShells.default = pkgs.mkShell rec {
        nativeBuildInputs = with pkgs; [
          (python3.withPackages (py: [
            py.pandas
            py.pyserial
            py.matplotlib
          ]))
        ];

        buildInputs = with pkgs; [

        ];

        LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath nativeBuildInputs;
      };
    });
}
