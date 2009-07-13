-- ~/.config/awesome/rc.lua

-- EXTENSIONS --
-- lmpd_plugin = assert(package.loadlib("/home/michael/.config/awesome/libluampd.so", "luaopen_lmpd"))

-- LIBRARIES --
require("awful")
require("beautiful")
require("naughty")
require("wicked")

-- INITIALIZATION
-- lmpd = lmpd_plugin()
-- lmpd.connect()

-- THEMES --
theme_path = "/home/michael/.config/awesome/themes/default/theme.lua"
beautiful.init(theme_path)

-- EXECUTABLES -- 
terminal = "urxvt"
editor = "kwrite"
editor_cmd = editor
browser = "midori"
irc = "quasselclient"
file_manager = "pcmanfm"

-- FUNCTIONS --
function increase_volume()
    os.execute("amixer sset Master 10%+")
    set_volume_widget()
end

function decrease_volume()
    os.execute("amixer sset Master 10%-")
    set_volume_widget()
end

function mute_volume()
    os.execute("amixer sset Master toggle")
    set_volume_widget()
end

function get_volume()
    return io.popen("amixer sget Master | grep -o \"[0-9]\\+%\""):read("*line")
end

function get_mute()
    local mutestring = io.popen("amixer sget Master | grep -o \"\\[\\(on\\|off\\)\\]\""):read("*line")
    local mute
    if mutestring == "[on]" then
        mute = false
    else
        mute = true
    end
    return mute
end

function set_volume_widget()
    local color
    if get_mute() then
        color = "#FF0000"
    else
        color = "#00FF00"
    end
    volumewidget.text = "<span color=\"white\">Volume:</span> <span color=\"" .. color .. "\">" .. get_volume() .. "</span>"
end

function battery_charging()
    local fh = io.popen("hal-get-property --udi /org/freedesktop/Hal/devices/computer_power_supply_battery_BAT1 --key battery.rechargeable.is_discharging")
    local chargingstring = fh:read("*line")
    fh:close()
    -- return image(beautiful.charging_icon)
    if chargingstring == "false" then
        return image(beautiful.charging_icon)
    else
        return image()
    end
end

function battery_level(format)
    local fh = io.popen("hal-get-property --udi /org/freedesktop/Hal/devices/computer_power_supply_battery_BAT1 --key battery.charge_level.percentage")
    local level = fh:read("*line")
    fh:close()
    local red = math.min(50, 100 - level) * 255 / 50
    local green = math.min(50, level) * 255 / 50
    color = string.format("#%02X%02X00", red, green)
    return {level, color}
end

function mpd_nowplaying(widget, args)
    if args[1]:find("volume:") == nil then
        return "<span color=\"white\">Now Playing:</span> " .. args[1]
    else
        return ""
    end
end

-- WIDGETS
mpdwidget = widget({type = "textbox", name = "mpdwidget", align = "right"})
volumewidget = widget({type = "textbox", name = "volumewidget", align = "right"})
batterywidget = widget({type = "textbox", name = "batterywidget", align = "right"})
batterychargingwidget = widget({type = "imagebox", name = "batterychargingwidget", align = "right"})
memwidget = widget({type = "textbox", name = "memwidget", align = "right"})
cpugraphwidget = widget({type = "graph", name = "cpugraphwidget", align = "right"})
cpuwidget = widget({type = "textbox", name = "cpuwidget", align = "right"})
datewidget = widget({type = "textbox", name = "datewidget", align = "right"})
systray = widget({ type = "systray", align = "right" })

volumewidget.width = 74
batterywidget.width = 88
cpugraphwidget:plot_properties_set("cpu", {
    fg = '#AEC6D8',
    fg_center = '#285577',
    fg_end = '#285577',
    vertical_gradient = false
})
cpuwidget.width = 70


    
-- wicked.register(mpdwidget, wicked.widgets.mpd, mpd_nowplaying, 1)
-- wicked.register(volumewidget, get_volume, " <span color=\"white\">Volume:</span> $1", 1)
wicked.register(batterywidget, battery_level, " <span color=\"white\">Battery:</span> <span color=\"$2\">$1%</span>", 1, nil)
wicked.register(memwidget, wicked.widgets.mem, " <span color=\"white\">Memory:</span> $1%", 1)
wicked.register(cpugraphwidget, wicked.widgets.cpu, "$1", 1, "cpu")
wicked.register(cpuwidget, wicked.widgets.cpu, " <span color=\"white\">CPU:</span> $1%", 1, nil, 3)
wicked.register(datewidget, wicked.widgets.date, " <span color=\"white\">Date:</span> %a %H:%M:%S ")
-- Default modkey.
-- Usually, Mod4 is the key with a logo between Control and Alt.
-- If you do not like this or do not have such a key,
-- I suggest you to remap Mod4 to another key using xmodmap or other tools.
-- However, you can use another modifier like Mod1, but it may interact with others.
modkey = "Mod4"

-- Table of layouts to cover with awful.layout.inc, order matters.
layouts =
{
    awful.layout.suit.tile,
    awful.layout.suit.tile.left,
    awful.layout.suit.tile.bottom,
    awful.layout.suit.tile.top,
    awful.layout.suit.fair,
    awful.layout.suit.fair.horizontal,
    awful.layout.suit.max,
    awful.layout.suit.max.fullscreen,
    awful.layout.suit.magnifier,
    awful.layout.suit.floating
}

-- Table of clients that should be set floating. The index may be either
-- the application class or instance. The instance is useful when running
-- a console app in a terminal like (Music on Console)
--    xterm -name mocp -e mocp
floatapps =
{
    -- by class
    ["MPlayer"] = true,
    ["pinentry"] = true,
    ["gimp"] = true,
    ["kopete"] = true
}

-- Applications to be moved to a pre-defined tag by class or instance.
-- Use the screen and tags indices.
apptags =
{
    -- ["Firefox"] = { screen = 1, tag = 2 },
    -- ["mocp"] = { screen = 2, tag = 4 },
}

-- Define if we want to use titlebar on all applications.
use_titlebar = false
-- }}}

-- {{{ Tags
-- Define tags table.
tags = {}
tags.settings =
{
    { name = "terminal", layout = layouts[1], },
    { name = "browsers", layout = layouts[7], },
    { name = "file management", layout = layouts[1] },
    { name = "mail", layout = layouts[7], },
    { name = "irc", layout = layouts[7], },
    { name = "im", layout = layouts[1], },
    { name = "editing", layout = layouts[1] },
    { name = "graphics", layout = layouts[7] },
    { name = "music", layout = layouts[7] },
    { name = "devel", layout = layouts[7] }
}
for s = 1, screen.count() do
    -- Each screen has its own tag table.
    tags[s] = {}
    -- Create 9 tags per screen.
    -- for tagnumber = 1, 9 do
    for tagnumber, taginfo in ipairs(tags.settings) do
        -- tags[s][tagnumber] = tag({name = tagnames[tagnumber][1], layout=tagnames[tagnumber][2]})
        tags[s][tagnumber] = tag(taginfo.name)
        -- Add tags to screen one by one
        tags[s][tagnumber].screen = s
        awful.layout.set(taginfo.layout, tags[s][tagnumber])
	-- awful.tag.setproperty(tags[s][tagnumber], "layout", taginfo.layout)
	-- awful.tag.setproperty(tags[s][tagnumber], "setslave", taginfo.setslave)
	-- awful.tag.setproperty(tags[s][tagnumber], "mwfact", taginfo.mwfact)
    end
    -- I'm sure you want to see at least one tag.
    tags[s][1].selected = true
end
-- }}}

-- {{{ Wibox
-- Create a textbox widget
mytextbox = widget({ type = "textbox", align = "right" })
-- Set the default text in textbox
mytextbox.text = "<b><small> " .. awesome.release .. " </small></b>"

-- Create a laucher widget and a main menu
myawesomemenu =
{
   { "manual", terminal .. " -e man awesome" },
   { "edit config", editor_cmd .. " " .. awful.util.getdir("config") .. "/rc.lua" },
   { "restart", awesome.restart },
   { "quit", awesome.quit }
}

mymainmenu = awful.menu.new({ items = {
    { "awesome", myawesomemenu, beautiful.awesome_icon },
    { "terminal", terminal },
    { "browser", browser },
    { "irc", irc },
    { "file manager", file_manager },
}})

mylauncher = awful.widget.launcher({ image = image(beautiful.awesome_icon),
                                     menu = mymainmenu })

-- Create a systray

-- Create a wibox for each screen and add it
mywibox = {}
mypromptbox = {}
mylayoutbox = {}
mytaglist = {}
mytaglist.buttons = awful.util.table.join(
                    awful.button({ }, 1, awful.tag.viewonly),
                    awful.button({ modkey }, 1, awful.client.movetotag),
                    awful.button({ }, 3, function (tag) tag.selected = not tag.selected end),
                    awful.button({ modkey }, 3, awful.client.toggletag),
                    awful.button({ }, 4, awful.tag.viewnext),
                    awful.button({ }, 5, awful.tag.viewprev)
                    )
mytasklist = {}
mytasklist.buttons = awful.util.table.join(
                     awful.button({ }, 1, function (c)
                                              if not c:isvisible() then
                                                  awful.tag.viewonly(c:tags()[1])
                                              end
                                              client.focus = c
                                              c:raise()
                                          end),
                     awful.button({ }, 3, function ()
                                              if instance then
                                                  instance:hide()
                                                  instance = nil
                                              else
                                                  instance = awful.menu.clients({ width=250 })
                                              end
                                          end),
                     awful.button({ }, 4, function ()
                                              awful.client.focus.byidx(1)
                                              if client.focus then client.focus:raise() end
                                          end),
                     awful.button({ }, 5, function ()
                                              awful.client.focus.byidx(-1)
                                              if client.focus then client.focus:raise() end
                                          end))

for s = 1, screen.count() do
    -- Create a promptbox for each screen
    mypromptbox[s] = awful.widget.prompt({ align = "left" })
    -- Create an imagebox widget which will contains an icon indicating which layout we're using.
    -- We need one layoutbox per screen.
    mylayoutbox[s] = awful.widget.layoutbox(s, { align = "right" })
    mylayoutbox[s]:buttons(awful.util.table.join(
                           awful.button({ }, 1, function () awful.layout.inc(layouts, 1) end),
                           awful.button({ }, 3, function () awful.layout.inc(layouts, -1) end),
                           awful.button({ }, 4, function () awful.layout.inc(layouts, 1) end),
                           awful.button({ }, 5, function () awful.layout.inc(layouts, -1) end)))
    -- Create a taglist widget
    mytaglist[s] = awful.widget.taglist(s, awful.widget.taglist.label.all, mytaglist.buttons)

    -- Create a tasklist widget
    mytasklist[s] = awful.widget.tasklist(function(c)
                                              return awful.widget.tasklist.label.currenttags(c, s)
                                          end, mytasklist.buttons)

    -- Create the wibox
    mywibox[s] = {}
    mywibox[s][1] = awful.wibox({ position = "top", screen = s })
    mywibox[s][2] = awful.wibox({ position = "bottom", screen = s })
    -- Add widgets to the wibox - order matters
    mywibox[s][1].widgets = {
        mylauncher,
        mytaglist[s],
        mypromptbox[s],
        mpdwidget,
        volumewidget,
        batterywidget,
        batterychargingwidget,
        memwidget,
        cpuwidget,
        cpugraphwidget,
        datewidget,
        s == 1 and systray or nil,
        mylayoutbox[s],
    }
    mywibox[s][2].widgets = {
        mytasklist[s],
    }
end
-- }}}

-- {{{ Mouse bindings
root.buttons(awful.util.table.join(
    awful.button({ }, 3, function () mymainmenu:toggle() end),
    awful.button({ }, 4, awful.tag.viewnext),
    awful.button({ }, 5, awful.tag.viewprev)
))
-- }}}

-- {{{ Key bindings
globalkeys = awful.util.table.join(
    awful.key({ modkey,           }, "Left",   awful.tag.viewprev       ),
    awful.key({ modkey,           }, "Right",  awful.tag.viewnext       ),
    awful.key({ modkey,           }, "Escape", awful.tag.history.restore),
    awful.key({}, "XF86AudioLowerVolume", decrease_volume),
    awful.key({}, "XF86AudioRaiseVolume", increase_volume),
    awful.key({}, "XF86AudioMute", mute_volume),
    --awful.key({}, "XF86AudioPlay", lmpd.play_pause),
    --awful.key({}, "XF86AudioStop", lmpd.stop),
    --awful.key({}, "XF86AudioPrev", lmpd.prev),
    --awful.key({}, "XF86AudioNext", lmpd.next),
    

    awful.key({ modkey,           }, "j",
        function ()
            awful.client.focus.byidx( 1)
            if client.focus then client.focus:raise() end
        end),
    awful.key({ modkey,           }, "k",
        function ()
            awful.client.focus.byidx(-1)
            if client.focus then client.focus:raise() end
        end),
    awful.key({ modkey,           }, "w", function () mymainmenu:show(true)        end),

    -- Layout manipulation
    awful.key({ modkey, "Shift"   }, "j", function () awful.client.swap.byidx(  1) end),
    awful.key({ modkey, "Shift"   }, "k", function () awful.client.swap.byidx( -1) end),
    awful.key({ modkey, "Control" }, "j", function () awful.screen.focus( 1)       end),
    awful.key({ modkey, "Control" }, "k", function () awful.screen.focus(-1)       end),
    awful.key({ modkey,           }, "u", awful.client.urgent.jumpto),
    awful.key({ modkey,           }, "Tab",
        function ()
            awful.client.focus.history.previous()
            if client.focus then
                client.focus:raise()
            end
        end),

    -- Standard program
    awful.key({ modkey,           }, "Return", function () awful.util.spawn(terminal) end),
    awful.key({ modkey, "Control" }, "r", awesome.restart),
    awful.key({ modkey, "Shift"   }, "q", awesome.quit),

    awful.key({ modkey,           }, "l",     function () awful.tag.incmwfact( 0.05)    end),
    awful.key({ modkey,           }, "h",     function () awful.tag.incmwfact(-0.05)    end),
    awful.key({ modkey, "Shift"   }, "h",     function () awful.tag.incnmaster( 1)      end),
    awful.key({ modkey, "Shift"   }, "l",     function () awful.tag.incnmaster(-1)      end),
    awful.key({ modkey, "Control" }, "h",     function () awful.tag.incncol( 1)         end),
    awful.key({ modkey, "Control" }, "l",     function () awful.tag.incncol(-1)         end),
    awful.key({ modkey,           }, "space", function () awful.layout.inc(layouts,  1) end),
    awful.key({ modkey, "Shift"   }, "space", function () awful.layout.inc(layouts, -1) end),

    -- Prompt
    awful.key({ modkey },            "r",     function () mypromptbox[mouse.screen]:run() end),

    awful.key({ modkey }, "x",
              function ()
                  awful.prompt.run({ prompt = "Run Lua code: " },
                  mypromptbox[mouse.screen].widget,
                  awful.util.eval, nil,
                  awful.util.getdir("cache") .. "/history_eval")
              end)
)

-- Client awful tagging: this is useful to tag some clients and then do stuff like move to tag on them
clientkeys = awful.util.table.join(
    awful.key({ modkey,           }, "f",      function (c) c.fullscreen = not c.fullscreen  end),
    awful.key({ modkey, "Shift"   }, "c",      function (c) c:kill()                         end),
    awful.key({ modkey, "Control" }, "space",  awful.client.floating.toggle                     ),
    awful.key({ modkey, "Control" }, "Return", function (c) c:swap(awful.client.getmaster()) end),
    awful.key({ modkey,           }, "o",      awful.client.movetoscreen                        ),
    awful.key({ modkey, "Shift"   }, "r",      function (c) c:redraw()                       end),
    awful.key({ modkey }, "t", awful.client.togglemarked),
    awful.key({ modkey,}, "m",
        function (c)
            c.maximized_horizontal = not c.maximized_horizontal
            c.maximized_vertical   = not c.maximized_vertical
        end)
)

-- Compute the maximum number of digit we need, limited to 9
keynumber = 0
for s = 1, screen.count() do
   keynumber = math.min(9, math.max(#tags[s], keynumber));
end

for i = 1, keynumber do
    globalkeys = awful.util.table.join(globalkeys,
        awful.key({ modkey }, i,
                  function ()
                        local screen = mouse.screen
                        if tags[screen][i] then
                            awful.tag.viewonly(tags[screen][i])
                        end
                  end),
        awful.key({ modkey, "Control" }, i,
                  function ()
                      local screen = mouse.screen
                      if tags[screen][i] then
                          tags[screen][i].selected = not tags[screen][i].selected
                      end
                  end),
        awful.key({ modkey, "Shift" }, i,
                  function ()
                      if client.focus and tags[client.focus.screen][i] then
                          awful.client.movetotag(tags[client.focus.screen][i])
                      end
                  end),
        awful.key({ modkey, "Control", "Shift" }, i,
                  function ()
                      if client.focus and tags[client.focus.screen][i] then
                          awful.client.toggletag(tags[client.focus.screen][i])
                      end
                  end),
        awful.key({ modkey, "Shift" }, "F" .. i,
                  function ()
                      local screen = mouse.screen
                      if tags[screen][i] then
                          for k, c in pairs(awful.client.getmarked()) do
                              awful.client.movetotag(tags[screen][i], c)
                          end
                      end
                   end))
end

-- Set keys
root.keys(globalkeys)
-- }}}

-- {{{ Hooks
-- Hook function to execute when focusing a client.
awful.hooks.focus.register(function (c)
    if not awful.client.ismarked(c) then
        c.border_color = beautiful.border_focus
    end
end)

-- Hook function to execute when unfocusing a client.
awful.hooks.unfocus.register(function (c)
    if not awful.client.ismarked(c) then
        c.border_color = beautiful.border_normal
    end
end)

-- Hook function to execute when marking a client
awful.hooks.marked.register(function (c)
    c.border_color = beautiful.border_marked
end)

-- Hook function to execute when unmarking a client.
awful.hooks.unmarked.register(function (c)
    c.border_color = beautiful.border_focus
end)

-- Hook function to execute when the mouse enters a client.
awful.hooks.mouse_enter.register(function (c)
    -- Sloppy focus, but disabled for magnifier layout
    if awful.layout.get(c.screen) ~= awful.layout.suit.magnifier
        and awful.client.focus.filter(c) then
        client.focus = c
    end
end)

-- Hook function to execute when a new client appears.
awful.hooks.manage.register(function (c, startup)
    -- If we are not managing this application at startup,
    -- move it to the screen where the mouse is.
    -- We only do it for filtered windows (i.e. no dock, etc).
    if not startup and awful.client.focus.filter(c) then
        c.screen = mouse.screen
    end

    if use_titlebar then
        -- Add a titlebar
        awful.titlebar.add(c, { modkey = modkey })
    end
    -- Add mouse bindings
    c:buttons(awful.util.table.join(
        awful.button({ }, 1, function (c) client.focus = c; c:raise() end),
        awful.button({ modkey }, 1, awful.mouse.client.move),
        awful.button({ modkey }, 3, awful.mouse.client.resize)
    ))
    -- New client may not receive focus
    -- if they're not focusable, so set border anyway.
    c.border_width = beautiful.border_width
    c.border_color = beautiful.border_normal

    -- Check if the application should be floating.
    local cls = c.class
    local inst = c.instance
    if floatapps[cls] ~= nil then
        awful.client.floating.set(c, floatapps[cls])
    elseif floatapps[inst] ~= nil then
        awful.client.floating.set(c, floatapps[inst])
    end

    -- Check application->screen/tag mappings.
    local target
    if apptags[cls] then
        target = apptags[cls]
    elseif apptags[inst] then
        target = apptags[inst]
    end
    if target then
        c.screen = target.screen
        awful.client.movetotag(tags[target.screen][target.tag], c)
    end

    -- Do this after tag mapping, so you don't see it on the wrong tag for a split second.
    client.focus = c

    -- Set key bindings
    c:keys(clientkeys)

    -- Set the windows at the slave,
    -- i.e. put it at the end of others instead of setting it master.
    -- awful.client.setslave(c)

    -- Honor size hints: if you want to drop the gaps between windows, set this to false.
    -- c.size_hints_honor = false
end)

-- Hook function to execute when switching tag selection.
awful.hooks.tags.register(function (screen, tag, view)
    -- Give focus to the latest client in history if no window has focus
    -- or if the current window is a desktop or a dock one.
    if not client.focus or not client.focus:isvisible() then
        local c = awful.client.focus.history.get(screen, 0)
        if c then client.focus = c end
    end
end)

-- Hook called every minute
awful.hooks.timer.register(60, function ()
    mytextbox.text = os.date(" %a %b %d, %H:%M ")
end)
awful.hooks.timer.register(1, function()
    batterychargingwidget.image = battery_charging()
end)
awful.hooks.timer.register(1, set_volume_widget)
-- }}}
