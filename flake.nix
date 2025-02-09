{
  description = "Generator for Geometric Inhomogeneous Random Graphs";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "girgs";
          version = "1.0.2";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
          ];

          buildInputs = with pkgs; [
            boost
            gtest
            doxygen
            graphviz
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DOPTION_BUILD_TESTS=ON"
            "-DOPTION_BUILD_EXAMPLES=ON"
            "-DOPTION_BUILD_CLI=ON"
          ];

          meta = with pkgs.lib; {
            description = "Generator for Geometric Inhomogeneous Random Graphs";
            homepage = "https://github.com/chistopher/girgs";
            license = {
              fullName = "MIT License";
              url = "https://opensource.org/licenses/MIT";
              spdxId = "MIT";
              file = ./LICENSE;
            };
            platforms = platforms.unix;
            maintainers = with maintainers; [ 
              (maintainers.lib.maintainer {
                name = "Dennis Kobert";
                email = "dennis@kobert.dev";
                github = "TrueDoctor";
              })
            ];
          };
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            cmake
            ninja
            boost
            gtest
            doxygen
            graphviz
            gcc
            gdb
            valgrind
            ccache
            clang-tools # For clang-format, clang-tidy
            pre-commit
          ];

          # Set environment variables for development
          shellHook = ''
            echo "Welcome to GIRGS development environment!"
            echo "Build tools and dependencies are available."
            
            # Setup ccache
            export CCACHE_DIR=$PWD/.ccache
            export PATH="${pkgs.ccache}/bin:$PATH"
            
            # Make tests verbose by default
            export CTEST_OUTPUT_ON_FAILURE=1
          '';
        };
      }
    );
}
