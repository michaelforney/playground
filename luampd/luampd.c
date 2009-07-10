#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <libmpd-1.0/libmpd/libmpd.h>
#include <libmpd-1.0/libmpd/debug_printf.h>

#include <lua.h>
#include <lauxlib.h>

#define L_MPD_MT "mpd.mpd_mt"

extern int debug_level;
void error_callback(MpdObj *mi,int errorid, char *msg, void *userdata)
{
	printf("Error %i: '%s'\n", errorid, msg);
} 

lua_State* GLOB_L;

void scriptCall(char* function)
{
        int top = lua_gettop(GLOB_L);
		printf("c tries to call lua function: %s\n", function);
        luaL_dostring(GLOB_L, function);
        lua_settop(GLOB_L, top);
}

void status_changed(MpdObj *mi, ChangedStatusType what)
{
	if(what&MPD_CST_SONGID)
	{
		mpd_Song *song = mpd_playlist_get_current_song(mi);
		if(song)
		{
			size_t length = strlen("mpd.display_song(\"\",\"\")") + strlen(song->artist) + strlen(song->title);
			char* tmp = malloc(length);
			sprintf(tmp, "mpd.display_song(\"%s\",\"%s\")", song->artist, song->title);
			assert(strlen(tmp) == length);
			scriptCall(tmp);
			free(tmp);
		}
	}

	if(what&MPD_CST_STATE)
	{
		printf("State:");
		switch(mpd_player_get_state(mi))
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
	if(what&MPD_CST_REPEAT){
		printf("Repeat: %s\n", mpd_player_get_repeat(mi)? "On":"Off");
	}
	if(what&MPD_CST_RANDOM){
		printf("Random: %s\n", mpd_player_get_random(mi)? "On":"Off");
	}
	if(what&MPD_CST_VOLUME){
		printf("Volume: %03i%%\n", 
				mpd_status_get_volume(mi));
	}
	if(what&MPD_CST_CROSSFADE){
		printf("X-Fade: %i sec.\n",
				mpd_status_get_crossfade(mi));
	}
	if(what&MPD_CST_UPDATING)
	{
		if(mpd_status_db_is_updating(mi))
		{
			printf("Started updating DB\n");
		}
		else
		{
			printf("Updating DB finished\n");
		}
	}
	if(what&MPD_CST_DATABASE)
	{
		printf("Databased changed\n");
	}
	if(what&MPD_CST_PLAYLIST)
	{
		printf("Playlist changed\n");
	}
	/* not yet implemented signals */
	if(what&MPD_CST_AUDIO){
		printf("Audio Changed\n");
	}
	if(what&MPD_CST_TOTAL_TIME){
		printf("Total song time changed: %02i:%02i\n",
				mpd_status_get_total_song_time(mi)/60,
				mpd_status_get_total_song_time(mi)%60);
	}
	if(what&MPD_CST_ELAPSED_TIME){
		if(what&MPD_CST_PERMISSION){
			printf("Permission: Changed\n");
		}
	}
}
int l_disconnect(lua_State* L, MpdObj* obj) {
	mpd_free(obj);
}

static MpdObj* connect(lua_State* L)
{
	MpdObj *obj = NULL;
	char* hostname;
	char *password;
	int iport;
	if(lua_gettop(L) == 3 && lua_isstring(L,1) 
			&& lua_isnumber(L,2)
			&& lua_isstring(L,3)) 
	{
		hostname=lua_tostring(L,1);
		iport = lua_tointeger(L,2);
		password = lua_tostring(L,3);
	} 
	else 
	{
		hostname = "localhost";
		iport = 6600;
		password = "";
	}
	/* Create mpd object */
	obj = mpd_new(hostname, iport,password); 
	/* Connect signals */
	mpd_signal_connect_error(obj,(ErrorCallback)error_callback, NULL);
	mpd_signal_connect_status_changed(obj,(StatusChangedCallback)status_changed, NULL);

	//mpd_set_connection_timeout(obj, 10);

	mpd_send_password(obj);
	GLOB_L = L;
	lua_pushboolean(L,mpd_connect(obj));
	return obj;
}

static int l_connect(lua_State* L)
{
	MpdObj *obj = connect(L);

		mpd_Song *song = mpd_playlist_get_current_song(obj);
		if(song)
		{
			size_t length = strlen("mpd.display_song(\"\",\"\")") + strlen(song->artist) + strlen(song->title);
			char* tmp = malloc(length);
			sprintf(tmp, "mpd.display_song(\"%s\",\"%s\")", song->artist, song->title);
			assert(strlen(tmp) == length);
			scriptCall(tmp);
			free(tmp);
		}

	GLOB_L = L;
	lua_pushboolean(L,!mpd_connect(obj));
	return 1;
}

void l_next(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_next(obj);
		mpd_status_update(obj);
	}
	l_disconnect(L,obj);
}

void l_prev(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_prev(obj);
		mpd_status_update(obj);
	}
	l_disconnect(L,obj);
}

void l_play(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_play(obj);
		mpd_status_update(obj);
	}
	l_disconnect(L,obj);
}

void l_pause(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_pause(obj);
		mpd_status_update(obj);
	}
	l_disconnect(L,obj);
}

void l_play_pause(lua_State* L) {
    MpdObj* obj = connect(L);
    if(!mpd_connect(obj)) {
        int status = mpd_player_get_state(obj);
        if(status == MPD_PLAYER_PLAY) {
            mpd_player_pause(obj);
        }
        else {
            mpd_player_play(obj);
        }
        mpd_status_update(obj);
    }
    l_disconnect(L,obj);
}

void l_stop(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_stop(obj);
		mpd_status_update(obj);
	}
	l_disconnect(L,obj);
}

void l_toggle_repeat(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_set_repeat(obj, !mpd_player_get_repeat(obj));
		mpd_status_update(obj);
	}
	l_disconnect(L,obj);
}

void l_toggle_random(lua_State* L) {
	MpdObj* obj = connect(L);
	if(!mpd_connect(obj)) {
		mpd_player_set_random(obj, !mpd_player_get_random(obj));
		mpd_status_update(obj);
	}
	l_disconnect(L, obj);
}


/* ------------------------------------------------------------------------
 * the class method table 
 */
static const luaL_reg class_table[] =
{
	{ "connect",		l_connect },
	{ "disconnect",		l_disconnect },
	{ "next",		l_next },
	{ "prev",		l_prev },
	{ "play",		l_play },
    { "play_pause", l_play_pause },
	{ "pause",		l_pause },
	{ "stop",		l_stop },
	{ "toggle_repeat",		l_toggle_repeat },
	{ "toggle_random",		l_toggle_random },
	
	{ NULL,			NULL },
};

/* ------------------------------------------------------------------------
 * the class metatable
 */
static int lel_init_mpd_class (lua_State *L)
{
	luaL_newmetatable(L, L_MPD_MT);

	// setup the __index and __gc field
	lua_pushstring (L, "__index");
	lua_pushvalue (L, -2);		// pushes the new metatable
	lua_settable (L, -3);		// metatable.__index = metatable

	//luaL_openlib (L, NULL, instance_table, 0);
	luaL_openlib (L, "mpd", class_table, 0);

	return 1;
}

/* ------------------------------------------------------------------------
 * library entry
 */
LUALIB_API int luaopen_lmpd (lua_State *L)
{
	return lel_init_mpd_class (L);
}
