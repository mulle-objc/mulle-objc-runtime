# Navigating Codebases with mulle-sde

## Quick Reference

```bash
# Setup
mulle-sde code doctor          # Check tool availability

# API Documentation
mulle-sde api list             # List available API docs
mulle-sde api cat <name>       # Read specific API doc
mulle-sde api apropos "query"  # Keyword search in docs

# Code Search
mulle-sde code grep "pattern"              # Fast text search
mulle-sde code grep --declarations "func"  # Find declarations only
mulle-sde code search <symbol>             # Semantic symbol search
mulle-sde code find header <file>          # Locate header file

# Symbol Analysis
mulle-sde code callers <symbol>   # Show who calls this
mulle-sde code callees <symbol>   # Show what this calls
mulle-sde code preflight <symbol> # Impact analysis
mulle-sde code map                # Project skeleton

# Symbol Listing
mulle-sde code symbol --functions  # List functions
mulle-sde symbol --json            # JSON output (shortcut)

# Direct Tool Access
mulle-sde code cs <args>    # Run cs directly
mulle-sde code roam <args>  # Run roam directly
```

## When to Use What

| Task | Command |
|------|---------|
| Find text/TODOs/errors | `code grep` |
| Find declarations only | `code grep --declarations` |
| Find symbol definitions | `code search` |
| Show callers/callees | `code callers/callees` |
| List all symbols | `code symbol` |
| Locate files | `code find` |
| Read dependency docs | `api cat` |
| Before modifying code | `code preflight` |
| Project overview | `code map` |

## Key Workflows

### Understanding New Code

```bash
mulle-sde api list                    # See dependencies
mulle-sde code map                    # Project structure
mulle-sde code symbol --public-headers # Public API
mulle-sde code understand             # AI overview
```

### Before Modifying a Symbol

```bash
mulle-sde code preflight mulle_malloc  # Risk assessment
mulle-sde code callers mulle_malloc    # Who depends on it
mulle-sde code callees mulle_malloc    # What it depends on
```

### Finding Usage Examples

```bash
mulle-sde api apropos "allocate"           # Find docs
mulle-sde code grep --declarations "malloc" # Find declarations
mulle-sde code grep --usages "malloc"       # Find usages
```

## Tool Details

### `mulle-sde api` - Documentation

Works with TOC.md files from crafted dependencies.

- `list [--all|--flat]` - Show available docs
- `cat <name>` - Display doc
- `apropos <query>` - Keyword search
- `context` - Dump all docs (for AI)

### `mulle-sde code` - Source Code

Works with project + all dependencies in stash.

**Search:**
- `grep <pattern>` - Fast text search (cs)
- `search <symbol>` - Semantic search (roam)
- `find <type> <name>` - Locate files

**Analysis:**
- `callers/callees/refs <symbol>` - Show relationships
- `preflight <symbol>` - Pre-change impact
- `map` - Project skeleton
- `understand` - AI overview

**Listing:**
- `symbol [opts]` - Extract symbols (ctags)

**Direct access:**
- `cs <args>` - Full cs control
- `roam <args>` - Full roam control

### Symbol Listing Options

```bash
# C symbols
--functions --structs --enums --typedefs --macros

# Objective-C symbols  
--interfaces --protocols --methods --properties

# Output formats
--json --csv -F '|'

# Scope
--public-headers (default) --headers --sources
```

## Tips

1. Start with `api list` and `code map`
2. Use `code callers` before refactoring
3. Use `--json` for scripting
4. Use `code cs` and `code roam` for full tool capabilities
5. Set `MULLE_VIBECODING=YES` for JSON-first output
