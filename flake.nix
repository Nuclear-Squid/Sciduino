{
  description = "Development flake for scpino1k board";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let pkgs = nixpkgs.legacyPackages.x86_64-linux; in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        (pkgs.python3.withPackages (pypkgs: with pypkgs; [ pyserial pyside6 numpy ]))
        arduino-cli

        kicad  # Electronics design suite

        ## Dev tools
        # Python
        ty    # Lsp and type checker
        ruff  # Linter and code formatter
      ];

      # Environment variable needed by QT to find the necessary libs and plugins
      NIXPKGS_QT6_QML_IMPORT_PATH = "${pkgs.qt6.full.outPath}/lib/qt-6/qml";
    };
  };
}
