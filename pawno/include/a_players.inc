
#if defined _players_included
	#endinput
#endif
#define _players_included

#define SPECIAL_ACTION_NONE				(0)
#define SPECIAL_ACTION_USEJETPACK		(2)
#define SPECIAL_ACTION_DANCE1			(5)
#define SPECIAL_ACTION_DANCE2			(6)
#define SPECIAL_ACTION_DANCE3			(7)
#define SPECIAL_ACTION_DANCE4			(8)
#define SPECIAL_ACTION_HANDSUP			(10)
#define SPECIAL_ACTION_USECELLPHONE		(11)
#define SPECIAL_ACTION_SITTING			(12)
#define SPECIAL_ACTION_STOPUSECELLPHONE (13)
#define SPECIAL_ACTION_URINATE			(68)

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
native SetPlayerAmmo(playerid, weaponslot, ammo);
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
native SetPlayerMapIcon(playerid, iconid, Float:x, Float:y, Float:z, typeid, color);
native RemovePlayerMapIcon(playerid, iconid);
native AllowPlayerTeleport(playerid, allow);
native SetPlayerCameraPos(playerid, Float:x, Float:y, Float:z, Float:rx=0.0, Float:ry=0.0, Float:rz=0.0);
native SetPlayerCameraLookAt(playerid, Float:x, Float:y, Float:z);
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
native SetPlayerArmedWeapon(playerid, weaponid);
native SetPlayerFightingStyle(playerid, style, moves = 6);
native SetPlayerMaxHealth(playerid, Float:max_health);
native SetPlayerBlurLevel(playerid,level);// level=0 off, level = 36 on 
native InterpolateCameraPos(playerid, Float:FromX, Float:FromY, Float:FromZ, Float:ToX, Float:ToY, Float:ToZ, time, cut = CAMERA_CUT);
native InterpolateCameraLookAt(playerid, Float:FromX, Float:FromY, Float:FromZ, Float:ToX, Float:ToY, Float:ToZ, time, cut = CAMERA_CUT);
