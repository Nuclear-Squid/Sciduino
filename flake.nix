{
  description = "Development flake for scpino1k board";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
      self-pkgs = self.packages.x86_64-linux;
      makeQtPluginPath = pkgList:
        builtins.concatStringsSep ":" (
          map (pkg: "${pkg.outPath}/lib/qt-6/pluigns") pkgList
        );
    in
      {
      packages.x86_64-linux.python = pkgs.python3.withPackages (pypkgs: with pypkgs; [
        pyserial
        pyside6
        pyqt6
      ]);

      devShells.x86_64-linux.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          self-pkgs.python
          arduino-cli
          qt6.full
          qtcreator
        ];

        # Environment variables needed by QT to find the necessary libs and plugins
        QT_PLUGIN_PATH = makeQtPluginPath [ pkgs.qt6.full self-pkgs.python ];
        NIXPKGS_QT6_QML_IMPORT_PATH = "${pkgs.qt6.full.outPath}/lib/qt-6/qml";
      };
    };
}
