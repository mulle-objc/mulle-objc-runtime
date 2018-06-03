# Used by `mulle-match find` to speed up the search.
export MULLE_MATCH_FIND_NAMES="config:*.h:*.inc:*.c:CMakeLists.txt:*.cmake"


# Used by `mulle-match find` to locate files
export MULLE_MATCH_FIND_LOCATIONS="${PROJECT_SOURCE_DIR}:CMakeLists.txt:cmake"


#
#
#
export MULLE_SDE_INSTALLED_VERSION="0.21.0"

# By default assume a project has source and a sourcetree
# The order should be sourcetree then source


# By default assume a project has source and a sourcetree in that order.
export MULLE_SDE_UPDATE_CALLBACKS="sourcetree:source"


