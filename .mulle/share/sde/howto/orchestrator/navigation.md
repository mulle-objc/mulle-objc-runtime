# Navigating Codebases with mulle-sde

## Quick reference

```bash
mulle-sde code doctor
mulle-sde api list
mulle-sde api cat <name>
mulle-sde api apropos "query"
mulle-sde code grep "pattern"
mulle-sde code search <symbol>
mulle-sde code callers <symbol>
mulle-sde code callees <symbol>
mulle-sde code preflight <symbol>
mulle-sde code map
```

## Good workflow

1. start with `api list` and `code map`
2. use `code callers` before refactoring
3. use `code preflight` before modifying important symbols
4. use `api apropos` and `code grep` for usage examples
