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

static MpdObj * checkmpd(lua_State * L)
{
    MpdObj ** mpd = (MpdObj **)luaL_checkudata(L, 1, "luampd");
    printf("Object pointer: %x\nObject: %x\n", mpd, *mpd);
    luaL_argcheck(L, mpd != NULL, 1, "'MpdObj *' expected");

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

static int connect(lua_State * L)
{
    printf("connect()\n");
    MpdObj * object = checkmpd(L);
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

static int disconnect(lua_State * L)
{
    printf("disconnect()\n");
    MpdObj * object = checkmpd(L);

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

static int next(lua_State * L)
{
    printf("next()\n");
    MpdObj * object = checkmpd(L);
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
    { "connect",        connect },
    { "disconnect",     disconnect },
    { "next",           next },
//     { "prev",           l_prev },
//     { "play",           l_play },
//     { "play_pause",     l_play_pause },
//     { "pause",          l_pause },
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

