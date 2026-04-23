{
  description = "Flake example for an mc-rtc controller with a superbuild shell";

  inputs = {
    mc-rtc-nix.url = "github:mc-rtc/nixpkgs";
    flake-parts.follows = "mc-rtc-nix/flake-parts";
    systems.follows = "mc-rtc-nix/systems";
    mc-rtc.url = "github:jrl-umi3218/mc_rtc/pull/526/head";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } (
      { ... }:
      {
        systems = import inputs.systems;
        imports = [
          inputs.mc-rtc-nix.flakeModule # if you don't need private repositories
          {
            flakoboros = {
              extraPackages = [ "ninja" ];

              overrideAttrs.mc-rtc =
              {
                src = inputs.mc-rtc;
              };

              packages = {
                mc-random-joints-example-controller =
                  {
                    stdenv,
                    cmake,
                    mc-rtc,
                    lib,
                    ...
                  }:
                  stdenv.mkDerivation {
                    name = "mc-random-joints-example-controller";
                    src = lib.cleanSource ./.;
                    nativeBuildInputs = [ cmake ];
                    propagatedBuildInputs = [
                      mc-rtc
                    ];

                    meta = with lib; {
                      description = "sample controller moving the joints randomly";
                      homepage = "https://github.com/arntanguy/mc-random-joints-example-controller";
                      license = licenses.bsd2;
                      platforms = platforms.all;
                    };
                  };
              };

              # Define a custom superbuild configuration
              # This will make all
              overrides.mc-rtc-superbuild-minimal =
                { pkgs-prev, pkgs-final, ... }:
                let
                  cfg-prev = pkgs-prev.mc-rtc-superbuild-minimal.superbuildArgs;
                in
                {
                  superbuildArgs = cfg-prev // {
                    pname = "mc-rtc-superbuild-override";
                    # for example, override any runtime dependency (robots, controllers, etc)
                    # # extend robots
                    robots = cfg-prev.robots ++ [ pkgs-final.mc-panda ];
                    # # override controllers
                    controllers = [ pkgs-final.mc-random-joints-example-controller ];
                    MainRobot = "PandaDefault";
                    Enabled = "RandomJointsExample";
                    Timestep = 0.001;
                    # plugins = [ pkgs-final.mc-force-shoe-plugin ];
                    # observers = [ pkgs-final.mc-state-observation ];
                    # apps = [];
                  };
                };

            };
          }
        ];
        perSystem =
          { pkgs, ... }:
          {
            # define a devShell called local-superbuild with the superbuild configuration above
            # you can also override attributes to add additional shell functionality
            devShells.default =
              (pkgs.callPackage "${inputs.mc-rtc-nix}/shell.nix" {
                mc-rtc-superbuild = pkgs.mc-rtc-superbuild-minimal;
              }).overrideAttrs
                (old: {
                  shellHook = ''
                    ${old.shellHook or ""}

                    echo "Welcome to the local superbuild devShell with the overridden mc-rtc-superbuild configuration!"
                  '';
                });
          };
      }
    );
}
