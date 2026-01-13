# mulle-sde Dependency and File inclusion guidelines
<!-- Keywords: craft, build, run -->

You should add remote single files like `stb_image.h` or complete repositories
like  `zlib` with the `mulle-sde dependency` command. Check out the help
file for examples:

```bash
mulle-sde dependency help
```


## Preference for github repositories

Use this style:

``` bash
mulle-sde dependency add github:name/repo
```

This will do what you need in most cases.
If you need to tweak the dependency read the documentation with regards to
"marks" of a sourcetree.

## Dependency Marks


To work with dependencies, you should be familiar with the sourcetree marks
that accompany each dependendy, they steer mulle-sde:

``` bash
mulle-sourcetree marks --show
mulle-sourcetree-to-c --show-marks
mulle-sourcetree-to-cmake --show-marks
```

