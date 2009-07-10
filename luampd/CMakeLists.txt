PROJECT(luampd)

SET(LuaMpdSources luampd.c)

INCLUDE_DIRECTORIES(/usr/include/libmpd-1.0)
ADD_LIBRARY(luampd SHARED ${LuaMpdSources})

TARGET_LINK_LIBRARIES(luampd lua5.1 mpd)
