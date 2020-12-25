
#if defined _players_included
	#endinput
#endif
#define _players_included

#define SPECIAL_ACTION_NONE				(0)
#define SPECIAL_ACTION_DUCK 			(1)
#define SPECIAL_ACTION_USEJETPACK		(2)
#define SPECIAL_ACTION_ENTER_VEHICLE 	(3)
#define SPECIAL_ACTION_EXIT_VEHICLE		(4)
#define SPECIAL_ACTION_DANCE1			(5)
#define SPECIAL_ACTION_DANCE2			(6)
#define SPECIAL_ACTION_DANCE3			(7)
#define SPECIAL_ACTION_DANCE4			(8)
#define SPECIAL_ACTION_HANDSUP			(10)
#define SPECIAL_ACTION_USECELLPHONE		(11)
#define SPECIAL_ACTION_SITTING			(12)
#define SPECIAL_ACTION_STOPUSECELLPHONE (13)
#define SPECIAL_ACTION_DRINK_BEER       (20)
#define SPECIAL_ACTION_URINATE			(68)
#define SPECIAL_ACTION_PISSING			(SPECIAL_ACTION_URINATE)

#define PLAYER_STATE_NONE						(0)
#define PLAYER_STATE_ONFOOT						(1)
#define PLAYER_STATE_DRIVER						(2)
#define PLAYER_STATE_PASSENGER					(3)
#define PLAYER_STATE_EXIT_VEHICLE				(4)
#define PLAYER_STATE_ENTER_VEHICLE_DRIVER		(5)
#define PLAYER_STATE_ENTER_VEHICLE_PASSENGER	(6)
#define PLAYER_STATE_WASTED						(7)
#define PLAYER_STATE_SPAWNED					(8)
#define PLAYER_STATE_SPECTATING					(9)

#define SPECTATE_TYPE_NONE		(0)
#define SPECTATE_TYPE_PLAYER	(1)
#define SPECTATE_TYPE_VEHICLE	(2)

#define WEAPONSKILL_PISTOL 			(0)
#define WEAPONSKILL_PISTOL_SILENCED	(1)
#define WEAPONSKILL_DESERT_EAGLE 	(2)
#define WEAPONSKILL_SHOTGUN 		(3)
#define WEAPONSKILL_SAWNOFF_SHOTGUN (4)
#define WEAPONSKILL_SPAS12_SHOTGUN 	(5)
#define WEAPONSKILL_MICRO_UZI 		(6)
#define WEAPONSKILL_MP5 			(7)
#define WEAPONSKILL_AK47 			(8)
#define WEAPONSKILL_M4 				(9)
#define WEAPONSKILL_SNIPERRIFLE 	(10)

#define CAMERA_MOVE (1)
#define CAMERA_CUT	(2)

#define WEAPONSTATE_UNKNOWN			(-1)
#define WEAPONSTATE_NO_BULLETS 		(0)
#define WEAPONSTATE_LAST_BULLET 	(1)
#define WEAPONSTATE_MORE_BULLETS	(2)
#define WEAPONSTATE_RELOADING 		(3)

#define PLAYER_VARTYPE_NONE		(0)
#define PLAYER_VARTYPE_INT		(1)
#define PLAYER_VARTYPE_STRING	(2)
#define PLAYER_VARTYPE_FLOAT	(3)

#define FIGHT_STYLE_NORMAL 		(4)
#define FIGHT_STYLE_BOXING 		(5)
#define FIGHT_STYLE_KUNGFU 		(6)
#define FIGHT_STYLE_KNEEHEAD 	(7)
#define FIGHT_STYLE_GRABKICK	(15)
#define FIGHT_STYLE_ELBOW 		(16)

#define KEY_ACTION 				(1)
#define KEY_CROUCH 				(2)
#define KEY_FIRE 				(4)
#define KEY_SPRINT 				(8)
#define KEY_SECONDARY_ATTACK	(16)
#define KEY_JUMP 				(32)
#define KEY_LOOK_RIGHT 			(64)
#define KEY_AIM 				(128)
#define KEY_HANDBRAKE 			(128)
#define KEY_LOOK_LEFT 			(256)
#define KEY_LOOK_BEHIND 		(512)
#define KEY_SUBMISSION 			(512)
#define KEY_WALK 				(1024)
#define KEY_ANALOG_UP 			(2048)
#define KEY_ANALOG_DOWN 		(4096)
#define KEY_ANALOG_LEFT 		(8192)
#define KEY_ANALOG_RIGHT 		(16384)
#define KEY_YES 				(32768)
#define KEY_NO 					(65536)
#define KEY_CTRL_BACK 			(131072)
#define KEY_CTRL_FORWARD 		(262144)
#define KEY_CAMERA 				(524288)
#define KEY_UP 					(-128)
#define KEY_DOWN 				(128)
#define KEY_LEFT 				(-128)
#define KEY_RIGHT 				(128)

#define MAPICON_LOCAL 				(0)
#define MAPICON_GLOBAL			 	(1)
#define MAPICON_LOCAL_CHECKPOINT 	(2)
#define MAPICON_GLOBAL_CHECKPOINT	(3)

native IsPickupStreamedIn(pickupid, forplayerid);
native GetPlayerCount();
native GetPlayerIDFromName(const name[]);
native SetSpawnInfo(playerid, teamid, skinid, Float:x, Float:y, Float:z, Float:a, weap1, ammo1, weap2, ammo2, weap3, ammo3);
native SpawnPlayer(playerid);
native SetPlayerPos(playerid, Float:x, Float:y, Float:z);
native SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z);
native GetPlayerPos(playerid, &Float:x, &Float:y, &Float:z);
native SetPlayerFacingAngle(playerid, Float:a);
native GetPlayerFacingAngle(playerid, &Float:a);
native IsPlayerInRangeOfPoint(playerid, Float:range, Float:x, Float:y, Float:z);
native Float:GetPlayerDistanceFromPoint(playerid, Float:X, Float:Y, Float:Z);
native SetPlayerInterior(playerid, interiorid);
native GetPlayerInterior(playerid);
native SetPlayerHealth(playerid, Float:health);
native GetPlayerHealth(playerid, &Float:health);
native SetPlayerArmour(playerid, Float:health);
native GetPlayerArmour(playerid, &Float:health);
native SetPlayerAmmo(playerid, weaponid, ammo);
native GetPlayerAmmo(playerid);
native SetPlayerTeam(playerid, teamid);
native GetPlayerTeam(playerid);
native SetPlayerScore(playerid, score);
native GetPlayerScore(playerid);
native GetPlayerDrunkLevel(playerid);
native SetPlayerDrunkLevel(playerid, level);
native SetPlayerColor(playerid, color);
native GetPlayerColor(playerid);
native SetPlayerSkin(playerid, skinid);
native GetPlayerSkin(playerid);
native GivePlayerWeapon(playerid, weaponid, ammo);
native ResetPlayerWeapons(playerid);
native GetPlayerWeaponData(playerid, slot, &weaponid, &ammo);
native GivePlayerMoney(playerid, money);
native ResetPlayerMoney(playerid);
native SetPlayerName(playerid, const name[]);
native GetPlayerMoney(playerid);
native SetPlayerMoney(playerid, money);
native GetPlayerState(playerid);
native GetPlayerIp(playerid, buffer[], length);
native GetPlayerPing(playerid);
native GetPlayerWeapon(playerid);
native GetPlayerKeys(playerid, &wkey, &ud, &lr);
native GetPlayerName(playerid, const buffer[], length);
native SetPlayerTime(playerid, hour, minute);
native GetPlayerTime(playerid, &hour, &minute);
native TogglePlayerClock(playerid, toggle);
native SetPlayerWeather(playerid, weatherid);
native ForceClassSelection(playerid);
native SetPlayerWantedLevel(playerid, level);
native GetPlayerWantedLevel(playerid);
native PutPlayerInVehicle(playerid, vehicleid, seatid);
native GetPlayerVehicleID(playerid);
native RemovePlayerFromVehicle(playerid);
native TogglePlayerControllable(playerid, toggle);
native PlayerPlaySound(playerid, soundid, Float:x, Float:y, Float:z);
native ApplyAnimation(playerid, const animlib[], const animname[], Float:speed, loop, lockx, locky, lockf, time);
native ClearAnimations(playerid);
native GetPlayerSpecialAction(playerid);
native SetPlayerSpecialAction(playerid, actionid);
native CreatePlayerPickup(pickupid, playerid, modelid, typeid, Float:x, Float:y, Float:z);
native DestroyPlayerPickup(pickupid, playerid);
native SetPlayerCheckpoint(playerid, Float:x, Float:y, Float:z, Float:radius);
native DisablePlayerCheckpoint(playerid);
native SetPlayerRaceCheckpoint(playerid, typeid, Float:x, Float:y, Float:z, Float:nextx, Float:nexty, Float:nextz, Float:radius);
native DisablePlayerRaceCheckpoint(playerid);
native SetPlayerWorldBounds(playerid, Float:maxx, Float:maxy, Float:minx, Float:miny);
native GetPlayerWorldBounds(playerid, &Float:maxx, &Float:maxy, &Float:minx, &Float:miny);
native SetPlayerMarkerForPlayer(playerid, forplayerid, color);
native ShowPlayerNameTagForPlayer(forplayerid, playerid, show);
native SetPlayerMapIcon(playerid, iconid, Float:x, Float:y, Float:z, markertype, color, style);
native RemovePlayerMapIcon(playerid, iconid);
native AllowPlayerTeleport(playerid, allow);
native SetPlayerCameraPos(playerid, Float:x, Float:y, Float:z, Float:rx=0.0, Float:ry=0.0, Float:rz=0.0);
native SetPlayerCameraLookAt(playerid, Float:x, Float:y, Float:z, cut = CAMERA_CUT);
native SetCameraBehindPlayer(playerid);
native IsPlayerConnected(playerid);
native IsPlayerInVehicle(playerid, vehicleid);
native IsPlayerInAnyVehicle(playerid);
native IsPlayerInCheckpoint(playerid);
native IsPlayerInRaceCheckpoint(playerid);
native SetPlayerVirtualWorld(playerid, worldid);
native GetPlayerVirtualWorld(playerid);
native EnableStuntBonusForPlayer(playerid, enable);
native EnableStuntBonusForAll(enable);
native TogglePlayerSpectating(playerid, toggle);
native PlayerSpectatePlayer(playerid, toplayerid, mode);
native PlayerSpectateVehicle(playerid, vehicleid, mode);
native CreateExplosionForPlayer(playerid, Float:x, Float:y, Float:z, type, Float:radius);
native SetPlayerVelocity(playerid, Float:X, Float:Y, Float:Z);
native GetPlayerVelocity(playerid, &Float:x, &Float:y, &Float:z);
native SetPlayerSkillLevel(playerid, skill, level);
native GetPlayerSurfingVehicleID(playerid);
native GetPlayerVehicleSeat(playerid);
native GetPlayerCameraMode(playerid);
native GetPlayerCameraZoom(playerid);
native GetPlayerCameraPos(playerid, &Float:x=0.0, &Float:y=0.0, &Float:z=0.0);
native GetPlayerCameraFrontVector(playerid, &Float:x=0.0, &Float:y=0.0, &Float:z=0.0);
native SetPlayerArmedWeapon(playerid, weaponid);
native GetPlayerFightingStyle(playerid);
native GetPlayerFightingMove(playerid);
native SetPlayerFightingStyle(playerid, style, moves = 6);
native SetPlayerMaxHealth(playerid, Float:max_health);
native InterpolateCameraPos(playerid, Float:FromX, Float:FromY, Float:FromZ, Float:ToX, Float:ToY, Float:ToZ, time, cut = CAMERA_CUT);
native InterpolateCameraLookAt(playerid, Float:FromX, Float:FromY, Float:FromZ, Float:ToX, Float:ToY, Float:ToZ, time, cut = CAMERA_CUT);
native SetPlayerBlurLevel(playerid, blur_level);
native SetPlayerGameSpeed(playerid, Float:speed);
native GetPlayerWeaponState(playerid);
native SendClientCheck(playerid, type, address, offset, count);
native TogglePlayerChatbox(playerid, show);
native IsPlayerTyping(playerid);
native TogglePlayerWidescreen(playerid, on);
native SetPlayerShopName(playerid, shopname[]);
native PlayAudioStreamForPlayer(playerid, url[], Float:posX = 0.0, Float:posY = 0.0, Float:posZ = 0.0, Float:distance = 50.0, usepos = 0);
native StopAudioStreamForPlayer(playerid);
native PlayCrimeReportForPlayer(playerid, suspectid, crime);

native SetPVarInt(playerid, varname[], int_value);
native SetPVarString(playerid, varname[], string_value[]);
native SetPVarFloat(playerid, varname[], Float:float_value);
native GetPVarInt(playerid, varname[]);
native GetPVarString(playerid, varname[], string_return[], len);
native Float:GetPVarFloat(playerid, varname[]);
native DeletePVar(playerid, varname[]);
native GetPVarType(playerid, varname[]);
native GetPVarNameAtIndex(playerid, index, ret_varname[], ret_len);
native GetPVarsUpperIndex(playerid);

// Per-player TextDraws
native PlayerText:CreatePlayerTextDraw(playerid, Float:x, Float:y, text[]);
native PlayerTextDrawLetterSize(playerid, PlayerText:text, Float:x, Float:y);
native PlayerTextDrawAlignment(playerid, PlayerText:text, alignment);
native PlayerTextDrawColor(playerid, PlayerText:text, color);
native PlayerTextDrawBoxColor(playerid, PlayerText:text, color);
native PlayerTextDrawBackgroundColor(playerid, PlayerText:text, color);
native PlayerTextDrawUseBox(playerid, PlayerText:text, use);
native PlayerTextDrawSetShadow(playerid, PlayerText:text, size);
native PlayerTextDrawFont(playerid, PlayerText:text, font);
native PlayerTextDrawSetOutline(playerid, PlayerText:text, size);
native PlayerTextDrawSetProportional(playerid, PlayerText:text, set);
native PlayerTextDrawShow(playerid, PlayerText:text);
native PlayerTextDrawHide(playerid, PlayerText:text);
native PlayerTextDrawSetString(playerid, PlayerText:text, string[]);
native PlayerTextDrawDestroy(playerid, PlayerText:text);

