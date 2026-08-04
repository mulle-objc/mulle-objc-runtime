# mulle-sde Advanced Dependency Management
<!-- Keywords: mulle-sde, dependency, addiction, tarball, archive, preload, craft -->

## Tarball preload

```bash
export DEPENDENCY_TARBALLS="/path/to/library.tar.gz:/path/to/another.tgz"
mulle-sde craft
```

## Addictions

Use `ADDICTION_DIR` for external preinstalled headers/libraries that should be
searched but not built by mulle-craft.

## Search path priority

1. dependency folder
2. addiction folder

Dependencies override addictions when both provide the same file.

## Debug dependency issues

```bash
mulle-sde dependency-dir
cat "$(mulle-sde dependency-dir)/etc/craftorder"
mulle-craft status
mulle-sde -DCMAKE_DEBUG_FLAGS=--debug-find recraft -v
```
