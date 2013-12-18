Bugfixed and improved HL release

URL: http://aghl.ru/forum/viewtopic.php?f=36&t=686

Version: 0.1.792

Info:

    Bugfixed and improved HLSDK release.
    This was started in a purpose to replace bugged hl.dll on a server to prevent frequent server crashes.
    Now it's a lot of fixes and improvements to server and client sides.
    Based on HLSDK version 2.3-p3 patched by metamod team.


Thanks:

    Valve for HLSDK release.
    Willday for his HLSDK patch.
    BubbleMod and Bigguy from hlpp.thewavelength.net for parts of spectator code.
    Uncle Mike from hlfx.ru for his Xash3D engine which was very helpful in hard moments.
    KORD_12.7 for constant helping and nice suggestions.
    AGHL.RU community for bug reporting and suggestions.


Installation:

    Copy all files from archive valve folder to server/client valve folder replacing all.
    For steam copy to Steam\steamapps\common\Half-Life folder.


Sources:

    You can get sources with any SVN client from https://svn.aghl.ru:8443/svn/HLSDK/trunk.
    Forum topic about them: Bugfixed and improved HLSDK (http://aghl.ru/forum/viewtopic.php?f=32&t=689).


Client side changes:

    0.1.223:
    Bunnyhop enabled.
    Fixed bug with mouse coordinates rounding.
    Fixed bug in displaying server name in score board: displaying garbage if server name was longer then 31 char.
    Fixed bug in displaying long server name in score board and MOTD.
    Colors for HP/AP points, new cvars hud_dim, hud_color, hud_color(1-3). New meaning for cvar hud_draw (values between 0-1 change hud transparency).
    Spectator mode improvement: removed top black panel and bottom is black only when menu is shown.
    Spectator mode improvement: Spectator camera menu now working.
    Clip mouse cursor to HL window so it will not go outside, which can lead to lost of focus on accidental click.
    Added DirectInput mouse processing. Added m_input cvar: 1 - standard windows cursor, 2 - DirectInput mouse.
    Fixed bug with mouse capturing on windows switching and on change level when window isn't active (engine bug).
    Fix for menu displayed on ShowMenu message (AMXX menu): hide menu on HudInit (change level, reconnect or connect to another server).
    Immediately output chat messages to console and prepend time for say messages.
    0.1.250:
    UTF Texts in hud menu and messages are now correctly displayed.
    Added validity checks into weapon hud menu display system.
    Extended weapon hud menu support up to 6 (0-5) weapon slots and 10 (0-9) positions in slots for new weapons (can be used in AMXX plugins).
    Fixed Score Board last killed by highlighting.
    0.1.253:
    Added loss to score board table.
    0.1.255:
    Added cvar: hud_showlossinscore - show/hide loss in score board.
    Changed column widths in score board.
    0.1.551 2012-11-23:
    Added "about" command to client.dll.
    Added exceptions catching and logging.
    Added mini-dump writing to exceptions handler.
    Added validity checks in team names processing code on client side.
    Fixed bug in LocaliseTextString function on a client, that caused buffer cut.
    Replaced texts in team menu that are absent in titles.txt file.
    Fixed bug with % in server name.
    Fixed various bugs in client-side with player on slot #32.
    Fixed bug with duplicate user message ids in engine, this eliminates bug with connecting to usual HLDM after AGmini server.
    Set dead players non-solid on client side to prevent lagging when step over them.
    Fixed mouse input rounding bug on network encoding.
    Default cl_pitchup and cl_pitchdown to 89.9999.
    Fixed various bugs in weapon sprite lists processing.
    Added loading of "damage by" sprites from weapon sprite lists (sprites starting with "d_").
    Now we can use custom sprites for "killed with" in deaths list.
    Egon beam life is extended a lot to prevent crashes in case of lags.
    Gait animation fixed for some player models (like from HD packs). Thanks to Uncle Mike from hlfx.ru.
    Engine bug fix: removed physics dependence on FPS.
    Added cvar engine_fix_fpsbug to enable/disable fix of fps bug.
    Ability to replace enemy and teammates models and colors.
    Added cvars: cl_forceemenymodels, cl_forceemenycolors, cl_forceteammatesmodel and cl_forceteammatescolors.
    Added "forcemodel" and "forcecolors" commands.
    Removed scrolling of empty area in scoreboard. Previously you could scroll all players up out of visible area.
    Slider in scoreboard now have length corresponding to visible area.
    Reset scroll position in scoreboard on changelevel.
    Fixed scrollbar hiding in scoreboard when all items fit the screen.
    Show steam IDs in scoreboard.
    Cut "STEAM_" and "VALVE_" from all steam ids, not only valid ones.
    Glide dimming of killer row highlighting in score board.
    Don't highlight row under mouse in scoreboard if mouse is within scrollbar.
    Capture mouse scrolling in scoreboard only if in squelch mode.
    Fixed mouse pressing and releasing with scoreboard.
    Squelch mode now turned ON only with left mouse button.
    Now scoreboard is less interfere with shooting.
    Added hud timer drawing.
    Added cvar "hud_timer".
    Retrieve mp_timelimit from Server Rules.
    Using mp_timeleft if mp_timelimit is giving wrong left time (due to agstart on miniAG server for example).
    Hide original miniAG timer.
    Hud timers info now written in demo stream and parsed on playback.
    Added "customtimer" command to the client.
    Display next map under the timer when last minute starts.
    Added cvar hud_nextmap to enable/disable nextmap show on last minute.
    Display nextmap in scoreboard.
    Catching amx_nextmap cvar changes.
    Timer synchronization don't block the thread.
    Results file names generation from results_file_format and results_counter_format cvars.
    Added saving snapshots to jpeg format.
    Added cvars: engine_snapshot_hook, snapshot_jpeg and snapshot_jpeg_quality.
    Write jpeg or bmp snapshots with reports file name generator.
    Added ability to log and dump user messages.
    Added cvar cl_messages_log: if not 0 client will log user messages in console when it will receive them from server.
    Added command cl_messages_dump: dump currently registered user messages with their ids and lengths.
    0.1.557 2012-11-26:
    Changed a bit usage info of customtimer command.
    Prediction for allow unstuck from a satchel feature.
    Prepend time in console for server say messages.
    Changed time format for say messages in console to [00:00:00].
    0.1.560 2012-11-27:
    Changed client-side unstuck code so it will not jerk in case of standing/moving/ducking on satchels or when stuck in a mine and trying to duck/jump/attack.
    0.1.579 2012-12-05:
    Errors from parsing overview files now goes to debug console (developer 1).
    Added PCRE library.
    Added hook for engine commands chain.
    Overrode and blocked motd_write command on the client.
    Added hook for SvcStuffText, SvcSendCvarValue and SvcSendCvarValue2 messages.
    Parsing commands in svc_stufftext and processing them with allow/block regex.
    Checking cvar in svc_sendcvarvalue and svc_sendcvarvalue2 against cvar block lists.
    Added client cvars cl_protect_block and cl_protect_allow to specify additional rules for server-to-client commands checking.
    Added client cvar cl_protect_block_cvar to specify additional rules for checking server requests for cvar values.
    Added client console command cl_protect that outputs some info about protection and built-in checking rules.
    Added client cl_protect_log to specify logging of commands issued and cvars accessed by server.
    Prevent commands execution via redirect connectionless packet.
    Fix for copy from the console problem, when it lose tail chars.
    0.1.595 2013-02-11:
    #10: Added parameters validation in TextMsg message processing.
    #4: Red killer row in score board fixed when previous killer is killed with worldspawn. Also properly take care of game time reset.
    #14: Rewritten memory patching so it will be more stable now and is compatible with some older clients (except for GameUI for now).
    #14: Added output messages to console if unable to apply memory patches.
    0.1.614 2013-02-13:
    Fixed scoreboard, overview and team/class menu problems with 32 player slot.
    #16: Fixed egon and gauss beams in first person spectator mode to go from weapon viewmodel (client-side change for all server types).
    #16: Do not calculate spectator movement on the client if in first person mode, this greatly reduce time lag of the movement.
    0.1.653 2013-03-28:
    Optimized unstuck table.
    Fixed crosshair in spectator mode.
    #6: Clear crosshair and ammo hud if all weapons are dropped (requires server-side fix too).
    Removed storing of hud_classautokill cvar value in setinfo buffer.
    Hud color remains the same if cvar value doesn't parse well.
    Default values for hud colors.
    Hud colors and hud_dim cvar values are archived to config.cfg.
    #4: Red killer row in score board fixed.
    Added client-side cvar cl_autowepswitch (requires server-side support for this to work).
    #13: Fixed model name checking and cleanup, preventing client crash on malformed model name.
    #12: Fixed crouched movement on a ladder, now crouching correctly reduce move speed and honor maxspeed setting.
    0.1.691 2013-04-06:
    Fixed viewmodel animation after weapon change.
    Fixed water level categorization for noclip move type and observer free look mode to correctly detect when feet are in floor and eyes are in water.
    0.1.700 2013-04-07:
    -
    0.1.717 2013-05-02:
    Set flashlight battery full on hud reset for better behavior on non-fixed server.
    0.1.746 2013-06-26:
    Get wider range health value from client state to draw it in HUD.
    Do battery(armor) value clamping on receive to 0-999 range.
    Apply wide range health on the client only if it is available (not a case for agmini spectate mode).
    Fixed health indication sprite alignment.
    Fixed critical bug in hud draw number function.
    Fixed death "d_" sprites loading from weapon sprites file.
    #22: Added cl_bunnyhop cvar to regulate client-side bunnyhop prediction.
    0.1.765 2013-07-07:
    Added cvar con_say_color to set color for say messages displayed in console with default of "30 230 50".
    Fixed snapshot and commands hooks on new steam version.
    #25: Added cvar hud_weapon to control displaying sprite of currently selected weapon.
    Show "(spectator)" tag after player names in score board for spectators.
    Send correct spec mode command from the spec menu, based on server version. This fixes spec menu for mini AG servers.
    0.1.780 2013-07-12:
    Send empty text for the center of the screen when going to intermission (fixes some bug in the engine that show text that was there last).
    Fixed spec mode sending for a case when server type isn't detected.
    #15: Added cvar hud_colortext to regulate color codes processing.
    #15: Processing color codes in hud, chat, death list, spectator panel, scoreboard, status bar.
    0.1.786 2013-07-29:
    Fixed view bug with cl_bobcycle 0.
    Added cvar hud_shownextmapinscore to show/hide nextmap in score board.
    0.1.790 2013-09-23:
    Fixed typo in cl_forceemenymodels cvar.
    Delayed "status" requesting on connect.
    0.1.792 2013-11-06:
    -


Server side changes:

    0.1.223:
    Bunnyhop enabled.
    Fixed bug with standing corpses and make dead player not solid so it will not lag other players passing over it.
    Fixed server crash when one player use tank that is used by other player doesn't having a weapon.
    Fix: now player will be respawned even if he holds a key pressed.
    Fixed bug when rocket detonate in air because of fuel out, launcher is stuck and not reloading.
    Workaround for bug with bouncing satchels causing high load in network channel and leading to choke. Further lowering net load by disabling animation of satchel model (its model has no animation, so nothing is lost). Corrected precache model for a satchel.
    Fixed: occasional crash after player leaved server while using tank.
    Integrated reversed spectator mode code.
    Change: Do 10000 damage on team switching instead of 900. This could be needed for some mods that sets more then 100 health to players.
    Say to all in chat about joining player.
    Fixed bug in FreeLook spectator mode when player is stick to a floor trying to move some angle upward along a plane.
    Remove deployed satchels on: death, disconnect, entering spectator mode and any other case where all weapons are stripped, even if not carry satchels control (dropped after deploying).
    More responsive status bar (targeted players names, HP, AP in teamplay) via more frequent updates.
    Fixed mp_fragsleft calculation for non-teamplay mode: Bug with 9999 when no one yet connected and bug with counting frags of disconnected players. Other bugs related to disconnected players counting.
    Set FCVAR_SERVER on allow_spectators so it is exported to rules.
    Spectator Mode:
    - Added leaving spectators feature.
    - Printing entering/leaving spectators events in chat.
    - Logging of entering/leaving spectators events.
    - Changed view modes cycle order: Roaming->FirstPerson->Chase Locked->Chase Free->Chase Overview->Free Overview.
    - Will spectate same target after returning to spectator mode.
    - Fixed: Don't switch from dead player to another if dead for more than 2 seconds. Just continue to spectate him.
    - Fixed: Bug when spectator has wrong visibility after target respawns. Now observers use the visibility of their target.
    - Fix: Allow selecting dead player as target for spectating.
    - Fix: Forcibly respawn on leaving spectator mode.
    - Made checks of spectating target more consistent.
    - Added check not allowing to spectate disconnected clients.
    - Fixed Bug: spectators shown as usual players in their teams if any player change the team.
    - Fixed StartDeathCam function. Now player will nicely overlook area after a death.
    - Enable "Cancel" button in team select menu (accessible via Enter when in spectator mode).
    - StopObserver function will always reset and respawn player.
    - Player will keep it's view point when entering Roaming spectator mode.
    - Added selection of view spot in Roaming spectator mode from "info_intermission", "info_player_coop", "info_player_start" and "info_player_deathmatch" entities.
    - Updates spectator's HUD in First Person mode with health/armor/weapon/ammo/pain info.
    - Even more responsive status bar: it will disappear instantly same as it appear.
    - Send AllowSpec message to enable Spectate command button in Team select panel.
    - Send Spectator message so plugins (in AMXX for example) could use them (it is not used in client DLL).
    - Made allow_spectators "1" by default.
    - Added feature to block frequent Spectator command usage. Default is 5 seconds delay.
    - Setup visibility of spectating target only for InEye mode.
    - Spectators reposition to target on its spawn - fixes PAS bug.
    - Show players in scoreboard in usual death match in two groups: players and spectators separately, show group of spectators separately in team death match mode. For death match mode and AMXX installed there should be su-27.amxx plugin installed to achieve described effect.
    - Spectators could use say_team to chat between them self.
    Fix for not storing previous weapon on auto switch upon pickup more powerful one. Now lastinv command can be used to switch back to weapon carried before pickup. Thanks to KORD_12.7.
    Host Say: no assertion anymore in debug configuration for UTF text, cut tail spaces in texts.
    Fixed camera bug: reset view entity on player spawn.
    Fix players detection as bots: Clear client flags in ClientDisconnect, because engine doesn't clear them before calling ClientConnect, but only before ClientPutInServer, on next connection to this slot. Thanks to Turanga_Leela.
    Remove think of weapon in touch event, preventing bug with further materialize and secondary attach. Also removes repeating thinking of weapons gave by default.
    Added missed WeapPickup messages to crowbar and glock.
    Fixed repeating switching between gauss and egon when no ammo.
    Let all know that new player has zero score (bug in scoreboard).
    0.1.250:
    Fixed rpg idle animation.
    Fixed fast repeating sound of empty reload of rpg on +attack holding.
    Fixed delay on primary attack with empty rpg launcher.
    Fix to don't show rpg laser spot when reloading.
    Added links to libm and libstdc++ for linux. Server now could be run with default hlds_run.
    0.1.257:
    Downgrading to gcc-2.95 under linux, this gives old style vtable placements and should solve offset problems for AMXX plugins and modules.
    0.1.258:
    Removed link to libstdc++ under linux. Let us see, may be it will work without it.
    0.1.484 2012-09-10:
    Refactored team names and list processing code in teamplay mode.
    Added validity check on model name set by player for incorrect characters like: <>:;%?*"|/\ and empty model name. This will also prevent different invisible model bug usages.
    Switch newly connected players to first person mode.
    Register VGUIMenu message on a server. This prevents bug with clients that came from miniAG server.
    Added "menuselect" client command processing to deathmatch mode. It do nothing, but will prevent "unknown command" reply.
    Fullupdate command flood check. No more then 2 commands within 5 second are allowed.
    Fixed chat flood control. Now it will check in burst mode: 3 messages within 0.5 sec with penalty of 2 seconds.
    Dedicated server will now run startup config file. Startup config filename is constructed by adding "startup_" string to filename specified in servercfgfile cvar.
    Fixed bug with satchels: when all satchel deployed and player standing on new ones for pickup, explode and after pickup you can't select satchels because satchels weapon is delayed stripped from player.
    Fixed fullupdate command to resend HideHUD, FOV, CurrentWeapon and Ammo info.
    Fixed UTIL_FindEntityInSphere function so that it will not return NULL on entity without class in code (so it will not stop searching entities on map entity from CS for example).
    Removed usages (only write usages existed) of "team" info buffer field.
    0.1.503 2012-09-28:
    Restore RemovePlayerItem virtual function signature, use some different way to destroy items on exaust (fixes problems with plugins that using it). Thanks to GordonFreeman at aghl.ru.
    0.1.533 2012-10-20:
    Spinning Corpses Fix (credits: omega from thewavelength.net).
    Added precaching of item_healthkit and ammo_9mmbox on world start so plugins may spawn them later. Thanks to KORD_12.7 at aghl.ru.
    Fixed bug in RetireWeapon when there is no another weapon to switch to. Thanks to Koshak at aghl.ru.
    0.1.544 2012-11-19:
    Prevent clients from using commands before PutInServer happen, i.e. before spawn. This will effectively block spambots.
    Register IconInfo message on a server. This prevents bug with clients that came from XDM server. Thanks to TesterYYY at aghl.ru.
    Register StatusIcon message on a server.
    0.1.549 2012-11-23:
    Added custom user messages from AGmini to prevent bug with clients that came from it.
    0.1.557 2012-11-26:
    Allow unstuck from a satchel, for example if player spawns on it.
    0.1.595 2013-02-11:
    Fixed quotes and tail spaces removal in say messages.
    #11: Fixed bug with single player: Weapons can't be changed in single player. Mark player as connected on restoring from save file.
    0.1.614 2013-02-13:
    #1: Added server-side cvar mp_notify_player_status to allow selecting notifications about join/leave/spectate.
    #16: Fixed egon beam in first person spectator mode to go from weapon viewmodel (server-side change for all client types).
    0.1.653 2013-03-28:
    Fixed delta.lst to correctly encode iuser1 and iuser2 player fields.
    Optimized unstuck table. Small unstuck movements now correctly applied, so player now can unstuck from a bugged lift.
    #9: Fixed ammo hud on client for spectator mode when changing targets.
    #9: Show correct ammo right after switch when switching targets in spectator mode.
    #6: Clear crosshair and ammo hud if all weapons are dropped (requires client-side fix too).
    Prevent "kill" command if client is already dead (for example in death cam mode) or is in spectator mode.
    Fixed bug with storing last pressed keys if client is dead or spectating.
    Fixed bug added with a patch against moving corpses, now view angles are correctly applying to dead or spectating players.
    Fixed buttons input in spectator mode to be "one click - one action".
    Fixed respawn time to always be not earlier then 1.5 seconds.
    Allow to rotate dead player angles after his body was copied, thus allowing spectators to see where he is looking.
    #18: Turn off player flashlight after death animation (1.5 seconds).
    Reverted spectator move predicting in first person mode, because this causing jerks on low ex_interp players.
    Fixed bug with non stopping move sound if door was blocked and returned back.
    Removed server fps dependence for CBaseToggle objects, like func_door, func_plat and etc. Fixes stuck in lifts on high server fps.
    Server now take into account client setting for auto weapon switching (cl_autowepswitch setinfo field).
    #13: Fixed model name checking and cleanup, preventing client crash on malformed model name.
    #12: Fixed crouched movement on a ladder, now crouching correctly reduce move speed and honor maxspeed setting.
    0.1.682 2013-04-06:
    Redone fix of bug with non stopping move sound if door was blocked and returned back: Do not stop a sound, but just do not start a new sound.
    Added player attack animation on primary shot from shotgun.
    Fixed bug with crossbow bolt when it continue to move after hit some entity which doesn't take damage and explode in some other point.
    Fixed tripmine big viewmodel bug when weapon prediction is turned off.
    Skip water move check for player in spectate mode.
    Fixed water level categorization for noclip move type and observer free look mode to correctly detect when feet are in floor and eyes are in water.
    Fixed stick to floor when moving upward in noclip mode.
    Check for "kill" command flood before other checks.
    Added IsBot and PutInServer fields for a player.
    Added "welcome cam" mode - player is put in semi-spectator mode on connect and can exit from it with +attack command.
    Added cvar mp_welcomecam to turn on/off welcome cam mode.
    Death cam now use intermission target to set angles, if any.
    0.1.691 2013-04-06:
    Fixed switch to spectator from welcome cam mode.
    0.1.700 2013-04-07:
    #21: Hide crosshair in welcome cam mode.
    Fixed move of doors, plats and trains to be correct after block happen.
    0.1.717 2013-05-02:
    Send flashlight status on game load.
    Fixed sound emitting by the plats when blocked.
    Changed stay time in top position of empty plat that bring a player to be the same as came without a player, 3 secs.
    Fixed latency in move of momentary_rot_button targets on retriggering.
    Fixed momentary_door destination approach algorithm.
    Fixed momentary_door sound bug.
    0.1.746 2013-06-26:
    Clamp health and armor values before delta compression and sending via user messages. This also prevents client screen tilt in some cases.
    Fixed health and armor value clamping. It will send max value if value is bigger then limit, so health will be send as 255 if higher.
    Increase max health transferred to client up to 1023 from 511 (delta.lst change). This will allow to display 999 in hud.
    Fixed sound emitting for touchable door when it is blocked.
    #23: Fixed targeted player name display in HUD when in first person spectator mode.
    Set clockwindow cvar to 0 in singleplayer to remove annoying delay at level load.
    Fixed listen server detection, so now correctly loading startup config for it.
    #22: Added cvar mp_bunnyhop to enable/disable bunnyhop on the server with default 0 in singleplayer and 1 in multiplayer.
    Prevent client kill command by not spawned player.
    Fixed bug for death notice when there is no inflictor.
    0.1.765 2013-07-07:
    Fixed health value clamping for passing via clientdata_t (prevents screen tilt bug).
    Send Spectator messages about players to newly connected player.
    0.1.780 2013-07-12:
    Send ammo, weapon and item pickup messages to spectators.
    #5: All four kinds of stationary weapons now do damage by means of using player, so they do add kills and respects mp_friendlyfire cvar. Same for explosive objects.
    Fixed func_tanklaser to stop its laser if player go away or die when fire from tank.
    0.1.786 2013-07-29:
    Fixed bug with explosions and breakable objects when there is no attacker.
    0.1.790 2013-09-23:
    -
    0.1.792 2013-11-06:
    Check that player object exists before marking as disconnected.

