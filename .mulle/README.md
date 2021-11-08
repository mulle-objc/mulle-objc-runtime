# .mulle

This `.mulle` folder is used by [mulle-sde](//mulle-sde.github.io) to
store project information.

## Structure

* `etc` is user editable, changes will be preserved.
* `share` is read only, changes will be lost on the next upgrade.
* `var` is ephemeral. You can delete and it will get recreated.

Every mulle-sde tool may have its own subfolder within those three folders.
It's name will be the name of the tool without the "mulle-" prefix.

You can edit the files in `etc` with any editor, but for consistency and
ease of use, it's usually better to use the appropriate mulle-sde tool.

## Remove .mulle

The share folder is often write protected, to prevent accidental user edits.

```
chmod -R ugo+rwX .mulle && rm -rf .mulle
```

