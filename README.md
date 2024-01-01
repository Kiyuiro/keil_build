
# UV4 Build and Flash Utility
This command-line utility automates the build and flashing process(by pyocd) for UV4 projects. It supports various options to customize the behavior of the build and flash operations.

## Usage
```bash
keil_build.exe [options]
```

## Options
* -u [UV4Path]: Specify the path to the UV4 executable.
* -b [BoardType]: Specify the board type.
* -h [HexPath]: Specify the path to the HEX file.
* -show: Show the UV4.exe GUI during the build process.
* -r: Enable rebuild, which forces a full project rebuild.
* -disup: Disable auto upload via pyocd after the build.

## Help
To view the help message:
```bash
keil_build.exe help
```

## Notice
You need to install pyocd and configure environment variables
