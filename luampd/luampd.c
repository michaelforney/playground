#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <libmpd/libmpd.h>
#include <libmpd/debug_printf.h>

#include <lua.h>
#include <lauxlib.h>

#define setfieldstring(A, B, C, D) lua_pushstring(A, D); lua_setfield(A, (B < 0) ? (B - 1) : (B), (C));
#define setfieldboolean(A, B, C, D) lua_pushboolean(A, D); lua_setfield(A, (B < 0) ? (B - 1) : (B), (C));
#define setfieldinteger(A, B, C, D) lua_pushinteger(A, D); lua_setfield(A, (B < 0) ? (B - 1) : (B), (C));

// typedef struct
// {
//     MpdObj * object;
//     const char * host;
//     int port;
//     const char * password;
// } mpd_connection;

int error_callback(MpdObj * object, int errorid, char * msg, void * userdata)
{
	printf("Error %i: '%s'\n", errorid, msg);
} 

void status_changed(MpdObj * object, ChangedStatusType type)
{
    if(type & MPD_CST_SONGID)
    {
        mpd_Song * song = mpd_playlist_get_current_song(object);
        if(song)
        {
            printf("Song Changed: %s - %s\n", song->title, song->artist);
            // do callback stuff
        }
    }
    if(type & MPD_CST_STATE)
    {
        printf("State:");
        switch(mpd_player_get_state(object))
        {
            case MPD_PLAYER_PLAY:
                printf("Playing\n");
                break;
            case MPD_PLAYER_PAUSE:
                printf("Paused\n");
                break;
            case MPD_PLAYER_STOP:
                printf("Stopped\n");
                break;
            default:
                break;
        }
    }
    if(type & MPD_CST_REPEAT)
    {
        printf("Repeat: %s\n", mpd_player_get_repeat(object)? "On":"Off");
    }
    if(type & MPD_CST_RANDOM)
    {
        printf("Random: %s\n", mpd_player_get_random(object)? "On":"Off");
    }
    if(type & MPD_CST_VOLUME)
    {
        printf("Volume: %03i%%\n", 
            mpd_status_get_volume(object));
    }
    if(type & MPD_CST_CROSSFADE)
    {
        printf("X-Fade: %i sec.\n",
            mpd_status_get_crossfade(object));
    }
    if(type & MPD_CST_UPDATING)
    {
        if(mpd_status_db_is_updating(object))
        {
            printf("Started updating DB\n");
        }
        else
        {
            printf("Updating DB finished\n");
        }
    }
    if(type & MPD_CST_DATABASE)
    {
        printf("Databased changed\n");
    }
    if(type & MPD_CST_PLAYLIST)
    {
        printf("Playlist changed\n");
    }
    /* not yet implemented signals */
    if(type & MPD_CST_AUDIO)
    {
        printf("Audio Changed\n");
    }
    if(type & MPD_CST_TOTAL_TIME)
    {
        printf("Total song time changed: %02i:%02i\n",
            mpd_status_get_total_song_time(object)/60,
            mpd_status_get_total_song_time(object)%60);
    }
    if(type & MPD_CST_ELAPSED_TIME)
    {
        if(type & MPD_CST_PERMISSION){
            printf("Perobjectssion: Changed\n");
        }
    }
}

static MpdObj * checkmpd(lua_State * L, int index)
{
    MpdObj ** mpd = (MpdObj **)luaL_checkudata(L, index, "luampd");
    luaL_argcheck(L, mpd != NULL, index, "'MpdObj *' expected");

    return *mpd;
}

static int newmpd(lua_State * L)
{
    int argcount = lua_gettop(L);
    if (argcount != 2 && argcount != 3)
    {
        lua_pushliteral(L, "new_mpd_connection requires 3 arguments");
        lua_error(L);
    }

    const char * host = luaL_checkstring(L, 1);
    int port = luaL_checkinteger(L, 2);
    const char * password = luaL_optstring(L, 3, "");

    luaL_argcheck(L, host != NULL, 1, "string expected");
    luaL_argcheck(L, port > 0, 2, "integer greater than 0 expected");

    MpdObj ** object_pointer = (MpdObj **)lua_newuserdata(L, sizeof(MpdObj *));

    luaL_getmetatable(L, "luampd");
    lua_setmetatable(L, -2);
    
    *object_pointer = mpd_new(host, port, password);
    MpdObj * object = *object_pointer;
    mpd_signal_connect_error(object, error_callback, NULL);
    mpd_connect(object);
    MpdError error = mpd_send_password(object);
    printf("Error id: %i\n", error);
    printf("Connected: %i\n", mpd_check_connected(object));
    printf("Object: %x\n", object);

    return 1;
}

static int luampd_connect(lua_State * L)
{
    printf("connect()\n");
    MpdObj * object = checkmpd(L, 1);
    MpdError error;

    if (mpd_check_connected(object))
    {
        printf("Already connected\n");
        lua_pushboolean(L, 1);
        return 1;
    }

    error = mpd_connect(object);
    error = mpd_send_password(object);
    lua_pushboolean(L, error == MPD_OK);
    return 1;
}

static int luampd_disconnect(lua_State * L)
{
    printf("disconnect()\n");
    MpdObj * object = checkmpd(L, 1);

    if (!mpd_check_connected(object))
    {
        printf("Already disconnected\n");
        lua_pushboolean(L, 1);
        return 1;
    }

    MpdError error = mpd_disconnect(object);
    lua_pushboolean(L, error == MPD_OK);
    return 1;
}

static int luampd_next(lua_State * L)
{
    printf("next()\n");
    MpdObj * object = checkmpd(L, 1);
    printf("Object: %x\n", object);
    MpdError error;

    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    error = mpd_player_next(object);
    printf("Status id: %i\n", error);
    mpd_status_update(object);

    lua_pushboolean(L, error == MPD_OK);
    return 1;
}

static int luampd_prev(lua_State * L)
{
    printf("prev()\n");
    MpdObj * object = checkmpd(L, 1);
    MpdError error;

    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    error = mpd_player_prev(object);
    mpd_status_update(object);

    lua_pushboolean(L, error == MPD_OK);
    return 1;
}

static int luampd_play(lua_State * L)
{
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    error = mpd_player_play(object);
    mpd_status_update(object);

    lua_pushboolean(L, error == MPD_OK);
}

static int luampd_pause(lua_State * L)
{
    printf("pause()\n");
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    error = mpd_player_pause(object);
    mpd_status_update(object);

    lua_pushboolean(L, error = MPD_OK);
}

static int luampd_stop(lua_State * L)
{
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    error = mpd_player_stop(object);
    mpd_status_update(object);

    lua_pushboolean(L, error == MPD_OK);
}

static int luampd_set_random(lua_State * L)
{
    printf("set_random()\n");
    MpdObj * object = checkmpd(L, 1);
    int enabled = lua_toboolean(L, 2);
    MpdError error;
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    printf("setting random to %i\n", enabled); 
    error = mpd_player_set_random(object, enabled);
    mpd_status_update(object);

    lua_pushboolean(L, error = MPD_OK);
}

static int luampd_get_random(lua_State * L)
{
    printf("get_random()\n");
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    mpd_status_update(object);
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushnil(L);
        return 1;
    }

    int enabled = mpd_player_get_random(object);
    printf("random is %i\n", enabled); 
    lua_pushboolean(L, enabled);
    return 1;
}

static int luampd_set_repeat(lua_State * L)
{
    printf("set_repeat()\n");
    MpdObj * object = checkmpd(L, 1);
    int enabled = lua_toboolean(L, 2);
    MpdError error;
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    printf("setting repeat to %i\n", enabled); 
    error = mpd_player_set_repeat(object, enabled);
    mpd_status_update(object);

    lua_pushboolean(L, error = MPD_OK);
}

static int luampd_get_repeat(lua_State * L)
{
    printf("get_repeat()\n");
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    mpd_status_update(object);
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushnil(L);
        return 1;
    }

    int enabled = mpd_player_get_repeat(object);
    printf("repeat is %i\n", enabled); 
    lua_pushboolean(L, enabled);
    return 1;
}

static int luampd_play_id(lua_State * L)
{
    printf("play_id()\n");
    MpdObj * object = checkmpd(L, 1);
    int id = luaL_checkinteger(L, 2);
    MpdError error;
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushboolean(L, 0);
        return 1;
    }

    printf("playing song id to %i\n", id);
    error = mpd_player_play_id(object, id);
    mpd_status_update(object);

    lua_pushboolean(L, error == MPD_OK);
    return 1;
}

int luampd_get_song(lua_State * L)
{
    printf("get_song()\n");
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    mpd_status_update(object);
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushnil(L);
        return 1;
    }

    if (mpd_player_get_state(object) != MPD_PLAYER_PLAY)
    {
        printf("not playing any song\n");
        lua_pushnil(L);
        return 1;
    }

    int id = mpd_player_get_current_song_id(object);
    printf("id is %i\n", id);
    mpd_Song * song = mpd_playlist_get_song(object, id);
    if (!song)
    {
        printf("Couldn't find song!!!!\n");
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    setfieldstring(L, -1, "file", song->file);
    setfieldstring(L, -1, "artist", song->artist);
    setfieldstring(L, -1, "title", song->title);
    setfieldstring(L, -1, "album", song->album);
    setfieldstring(L, -1, "track", song->track);
    setfieldstring(L, -1, "name", song->name);
    setfieldstring(L, -1, "date", song->date);
    setfieldstring(L, -1, "genre", song->genre);
    setfieldstring(L, -1, "composer", song->composer);
    setfieldstring(L, -1, "performer", song->performer);
    setfieldstring(L, -1, "disc", song->disc);
    setfieldstring(L, -1, "comment", song->comment);
    setfieldstring(L, -1, "albumartist", song->albumartist);
    setfieldinteger(L, -1, "time", song->time);
    setfieldinteger(L, -1, "pos", song->pos);
    setfieldinteger(L, -1, "id", song->id);
    mpd_freeSong(song);
    return 1;
}

static int luampd_get_status(lua_State * L)
{
    MpdObj * object = checkmpd(L, 1);
    MpdError error;
    mpd_status_update(object);
    if (!mpd_check_connected(object))
    {
        printf("not connected\n");
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    setfieldinteger(L, -1, "volume", mpd_status_get_volume(object));
    // setfieldboolean(L, -1, "repeat", mpd_status_get_repeat(object));
    // setfieldboolean(L, -1, "random", mpd_status_get_random(object));
    // setfieldinteger(L, -1, "playlistLength", mpd_status_get_playlistLength(object));
    // setfieldinteger(L, -1, "playlist", mpd_status_get_playlist(object));
    // setfieldinteger(L, -1, "storedplaylist", mpd_status_get_storedplaylist(object));
    // setfieldinteger(L, -1, "state", mpd_status_get_state(object));
    setfieldinteger(L, -1, "crossfade", mpd_status_get_crossfade(object));
    // setfieldinteger(L, -1, "song", mpd_status_get_song(object));
    // setfieldinteger(L, -1, "songid", mpd_status_get_songid(object));
    setfieldinteger(L, -1, "elapsed", mpd_status_get_elapsed_song_time(object));
    setfieldinteger(L, -1, "total", mpd_status_get_total_song_time(object));
    setfieldinteger(L, -1, "bitrate", mpd_status_get_bitrate(object));
    setfieldinteger(L, -1, "samplerate", mpd_status_get_samplerate(object));
    setfieldinteger(L, -1, "bits", mpd_status_get_bits(object));
    setfieldinteger(L, -1, "channels", mpd_status_get_channels(object));
    setfieldboolean(L, -1, "updating_db", mpd_status_db_is_updating(object));
    setfieldstring(L, -1, "error", mpd_status_get_mpd_error(object));
    return 1;
}

// int l_prev(lua_State* L) {
// 	MpdObj* obj = connect(L);
// 	if(!mpd_connect(obj)) {
// 		mpd_player_prev(obj);
// 		mpd_status_update(obj);
// 	}
// 	l_disconnect(L,obj);
// }
// 
// int l_play(lua_State* L) {
// 	MpdObj* obj = connect(L);
// 	if(!mpd_connect(obj)) {
// 		mpd_player_play(obj);
// 		mpd_status_update(obj);
// 	}
// 	l_disconnect(L,obj);
// }
// 
// int l_pause(lua_State* L) {
// 	MpdObj* obj = connect(L);
// 	if(!mpd_connect(obj)) {
// 		mpd_player_pause(obj);
// 		mpd_status_update(obj);
// 	}
// 	l_disconnect(L,obj);
// }
// 
// int l_play_pause(lua_State* L) {
//     MpdObj* obj = connect(L);
//     if(!mpd_connect(obj)) {
//         int status = mpd_player_get_state(obj);
//         if(status == MPD_PLAYER_PLAY) {
//             mpd_player_pause(obj);
//         }
//         else {
//             mpd_player_play(obj);
//         }
//         mpd_status_update(obj);
//     }
//     l_disconnect(L,obj);
// }
// 
// int l_stop(lua_State* L) {
// 	MpdObj* obj = connect(L);
// 	if(!mpd_connect(obj)) {
// 		mpd_player_stop(obj);
// 		mpd_status_update(obj);
// 	}
// 	l_disconnect(L,obj);
// }
// 
// int l_toggle_repeat(lua_State* L) {
// 	MpdObj* obj = connect(L);
// 	if(!mpd_connect(obj)) {
// 		mpd_player_set_repeat(obj, !mpd_player_get_repeat(obj));
// 		mpd_status_update(obj);
// 	}
// 	l_disconnect(L,obj);
// }
// 
// int l_toggle_random(lua_State* L) {
// 	MpdObj* obj = connect(L);
// 	if(!mpd_connect(obj)) {
// 		mpd_player_set_random(obj, !mpd_player_get_random(obj));
// 		mpd_status_update(obj);
// 	}
// 	l_disconnect(L, obj);
// }

static const luaL_reg luampd_mreg[] =
{
    { "connect",        luampd_connect },
    { "disconnect",     luampd_disconnect },
    { "next",           luampd_next },
    { "prev",           luampd_prev },
    { "play",           luampd_play },
    { "stop",           luampd_stop },
//     { "play_pause",     l_play_pause },
    { "pause",          luampd_pause },
    { "set_random",     luampd_set_random },
    { "get_random",     luampd_get_random },
    { "set_repeat",     luampd_set_repeat },
    { "get_repeat",     luampd_get_repeat },
    { "play_id",        luampd_play_id },
    { "get_song",       luampd_get_song },
    { "get_status",     luampd_get_status },
//     { "stop",           l_stop },
//     { "toggle_repeat",  l_toggle_repeat },
//     { "toggle_random",  l_toggle_random },

    { NULL,             NULL },
};

static const luaL_reg luampd_freg[] =
{
    { "new",        newmpd },

    { NULL,             NULL },
};

LUALIB_API int luaopen_luampd (lua_State *L)
{
    luaL_newmetatable(L, "luampd");

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_register(L, NULL, luampd_mreg);

    luaL_register(L, "mpd", luampd_freg);
    return 1;
}

