{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    nativeBuildInputs = with pkgs; [
        gnumake
        libllvm
        pkg-config
        meson
        ninja
        cmake
        emscripten
        python314
        binaryen
        git
        bash
        autoconf
        automake
        libtool
        unzip
        zip
        javaPackages.compiler.openjdk25
        alsa-lib
        alsa-oss
        cups
        fontconfig
        xorg.libX11
        xorg.libXinerama
        xorg.libXext
        xorg.libXcursor
        xorg.libXtst
        xorg.libXt
        xorg.libXinerama
        xorg.libXext
        xorg.libXcursor
        xorg.libXrender
        xorg.libXrandr
        xorg.xorgproto
        wabt

        ant
    ];

    shellHook = ''
      export ANT_HOME="${pkgs.ant}/share/ant"
      export JAVA_HOME="${pkgs.jdk.home}"
    '';
}