
#if defined _samp_included
	#endinput
#endif
#define _samp_included

#pragma tabsize 4

#include <core>
#include <float>
#include <string>
#include <file>
#include <time>
#include <a_players>
#include <a_vehicles>
#include <a_objects>
#include <a_sampdb>
#include <a_http>

#define MAX_PLAYER_NAME		(24)
#define MAX_PLAYERS			(200)
#define MAX_VEHICLES		(700)
#define INVALID_PLAYER_ID	(255)
#define INVALID_VEHICLE_ID 	(0xFFFF)
#define NO_TEAM 			(255)
#define MAX_OBJECTS			(255)
#define INVALID_OBJECT_ID	(255)
#define MAX_GANG_ZONES 		(1024)
#define MAX_TEXT_DRAWS 		(1024)
#define MAX_MENUS 			(128)
#define MAX_MENU_ITEMS		(12)
#define MAX_MENU_LINE		(32)
#define MAX_COLUMNS 		(2)
#define MAX_PICKUPS 		(400)
#define INVALID_MENU		(255)
#define INVALID_TEXT_DRAW	(0xFFFF)
#define INVALID_GANG_ZONE	(0xFFFF)
#define MAX_CMD_INPUT 		(128)
#define MAX_TEXT_DRAW_LINE	(256)
#define MAX_FILTER_SCRIPTS 	(16)
#define MAX_MAP_ICON		(200)

#define DISCONNECT_REASON_LOST_CONNECTION	(0)
#define DISCONNECT_REASON_DISCONNECTED 		(1)
#define DISCONNECT_REASON_KICKED 			(2)

#define WEAPON_FIST 					(0)
#define WEAPON_BRASSKNUCKLE				(1)
#define WEAPON_GOLFCLUB					(2)
#define WEAPON_NITESTICK				(3)
#define WEAPON_NIGHTSTICK				(3)
#define WEAPON_KNIFE					(4)
#define WEAPON_BAT						(5)
#define WEAPON_SHOVEL					(6)
#define WEAPON_POOLSTICK				(7)
#define WEAPON_KATANA					(8)
#define WEAPON_CHAINSAW					(9)
#define WEAPON_DILDO					(10)
#define WEAPON_DILDO2					(11)
#define WEAPON_VIBRATOR					(12)
#define WEAPON_VIBRATOR2				(13)
#define WEAPON_FLOWER					(14)
#define WEAPON_CANE						(15)
#define WEAPON_GRENADE					(16)
#define WEAPON_TEARGAS					(17)
#define WEAPON_MOLTOV					(18)
#define WEAPON_MOLOTOV					(18)
#define WEAPON_ROCKET					(19)
#define WEAPON_ROCKET_HS				(20)
#define WEAPON_FREEFALLBOMB				(21)
#define WEAPON_COLT45					(22)
#define WEAPON_SILENCED					(23)
#define WEAPON_DEAGLE					(24)
#define WEAPON_SHOTGUN					(25)
#define WEAPON_SAWEDOFF					(26)
#define WEAPON_SHOTGSPA					(27)
#define WEAPON_UZI						(28)
#define WEAPON_MP5						(29)
#define WEAPON_AK47						(30)
#define WEAPON_M4						(31)
#define WEAPON_TEC9						(32)
#define WEAPON_RIFLE					(33)
#define WEAPON_SNIPER					(34)
#define WEAPON_ROCKETLAUNCHER			(35)
#define WEAPON_HEATSEEKER				(36)
#define WEAPON_FLAMETHROWER				(37)
#define WEAPON_MINIGUN					(38)
#define WEAPON_SATCHEL					(39)
#define WEAPON_BOMB						(40)
#define WEAPON_SPRAYCAN					(41)
#define WEAPON_FIREEXTINGUISHER			(42)
#define WEAPON_CAMERA					(43)
#define WEAPON_NIGHTVISION				(44)
#define WEAPON_INFRARED					(45)
#define WEAPON_PARACHUTE				(46)
#define WEAPON_ARMOUR					(47)
#define WEAPON_VEHICLE					(49)
#define WEAPON_HELIBLADES				(50)
#define WEAPON_EXPLOSION				(51)
#define WEAPON_DROWN					(53)
#define WEAPON_COLLISION				(54)
#define WEAPON_CONNECT 					(200)
#define WEAPON_DISCONNECT 				(201)
#define WEAPON_SUICIDE 					(255)

// Source: https://gist.github.com/mick88/caf27f31afe68c7199ac
#define VEHICLE_LANDSTAL    (400)
#define VEHICLE_BRAVURA     (401)
#define VEHICLE_BUFFALO     (402)
#define VEHICLE_LINERUN     (403)
#define VEHICLE_PEREN       (404)
#define VEHICLE_SENTINEL    (405)
#define VEHICLE_DUMPER      (406)
#define VEHICLE_FIRETRUK    (407)
#define VEHICLE_TRASH       (408)
#define VEHICLE_STRETCH     (409)
#define VEHICLE_MANANA      (410)
#define VEHICLE_INFERNUS    (411)
#define VEHICLE_VOODOO      (412)
#define VEHICLE_PONY        (413)
#define VEHICLE_MULE        (414)
#define VEHICLE_CHEETAH     (415)
#define VEHICLE_AMBULAN     (416)
#define VEHICLE_LEVIATHN    (417)
#define VEHICLE_MOONBEAM    (418)
#define VEHICLE_ESPERANT    (419)
#define VEHICLE_TAXI        (420)
#define VEHICLE_WASHING     (421)
#define VEHICLE_BOBCAT      (422)
#define VEHICLE_MRWHOOP     (423)
#define VEHICLE_BFINJECT    (424)
#define VEHICLE_HUNTER      (425)
#define VEHICLE_PREMIER     (426)
#define VEHICLE_ENFORCER    (427)
#define VEHICLE_SECURICA    (428)
#define VEHICLE_BANSHEE     (429)
#define VEHICLE_PREDATOR    (430)
#define VEHICLE_BUS         (431)
#define VEHICLE_RHINO       (432)
#define VEHICLE_BARRACKS    (433)
#define VEHICLE_HOTKNIFE    (434)
#define VEHICLE_ARTICT1     (435)
#define VEHICLE_PREVION     (436)
#define VEHICLE_COACH       (437)
#define VEHICLE_CABBIE      (438)
#define VEHICLE_STALLION    (439)
#define VEHICLE_RUMPO       (440)
#define VEHICLE_RCBANDIT    (441)
#define VEHICLE_ROMERO      (442)
#define VEHICLE_PACKER      (443)
#define VEHICLE_MONSTER     (444)
#define VEHICLE_ADMIRAL     (445)
#define VEHICLE_SQUALO      (446)
#define VEHICLE_SEASPAR     (447)
#define VEHICLE_PIZZABOY    (448)
#define VEHICLE_TRAM        (449)
#define VEHICLE_ARTICT2     (450)
#define VEHICLE_TURISMO     (451)
#define VEHICLE_SPEEDER     (452)
#define VEHICLE_REEFER      (453)
#define VEHICLE_TROPIC      (454)
#define VEHICLE_FLATBED     (455)
#define VEHICLE_YANKEE      (456)
#define VEHICLE_CADDY       (457)
#define VEHICLE_SOLAIR      (458)
#define VEHICLE_TOPFUN      (459)
#define VEHICLE_SKIMMER     (460)
#define VEHICLE_PCJ600      (461)
#define VEHICLE_FAGGIO      (462)
#define VEHICLE_FREEWAY     (463)
#define VEHICLE_RCBARON     (464)
#define VEHICLE_RCRAIDER    (465)
#define VEHICLE_GLENDALE    (466)
#define VEHICLE_OCEANIC     (467)
#define VEHICLE_SANCHEZ     (468)
#define VEHICLE_SPARROW     (469)
#define VEHICLE_PATRIOT     (470)
#define VEHICLE_QUAD        (471)
#define VEHICLE_COASTG      (472)
#define VEHICLE_DINGHY      (473)
#define VEHICLE_HERMES      (474)
#define VEHICLE_SABRE       (475)
#define VEHICLE_RUSTLER     (476)
#define VEHICLE_ZR350       (477)
#define VEHICLE_WALTON      (478)
#define VEHICLE_REGINA      (479)
#define VEHICLE_COMET       (480)
#define VEHICLE_BMX         (481)
#define VEHICLE_BURRITO     (482)
#define VEHICLE_CAMPER      (483)
#define VEHICLE_MARQUIS     (484)
#define VEHICLE_BAGGAGE     (485)
#define VEHICLE_DOZER       (486)
#define VEHICLE_MAVERICK    (487)
#define VEHICLE_VCNMAV      (488)
#define VEHICLE_RANCHER     (489)
#define VEHICLE_FBIRANCH    (490)
#define VEHICLE_VIRGO       (491)
#define VEHICLE_GREENWOO    (492)
#define VEHICLE_JETMAX      (493)
#define VEHICLE_HOTRING     (494)
#define VEHICLE_SANDKING    (495)
#define VEHICLE_BLISTAC     (496)
#define VEHICLE_POLMAV      (497)
#define VEHICLE_BOXVILLE    (498)
#define VEHICLE_BENSON      (499)
#define VEHICLE_MESA        (500)
#define VEHICLE_RCGOBLIN    (501)
#define VEHICLE_HOTRINA     (502)
#define VEHICLE_HOTRINB     (503)
#define VEHICLE_BLOODRA     (504)
#define VEHICLE_RNCHLURE    (505)
#define VEHICLE_SUPERGT     (506)
#define VEHICLE_ELEGANT     (507)
#define VEHICLE_JOURNEY     (508)
#define VEHICLE_BIKE        (509)
#define VEHICLE_MTBIKE      (510)
#define VEHICLE_BEAGLE      (511)
#define VEHICLE_CROPDUST    (512)
#define VEHICLE_STUNT       (513)
#define VEHICLE_PETRO       (514)
#define VEHICLE_RDTRAIN     (515)
#define VEHICLE_NEBULA      (516)
#define VEHICLE_MAJESTIC    (517)
#define VEHICLE_BUCCANEE    (518)
#define VEHICLE_SHAMAL      (519)
#define VEHICLE_HYDRA       (520)
#define VEHICLE_FCR900      (521)
#define VEHICLE_NRG500      (522)
#define VEHICLE_COPBIKE     (523)
#define VEHICLE_CEMENT      (524)
#define VEHICLE_TOWTRUCK    (525)
#define VEHICLE_FORTUNE     (526)
#define VEHICLE_CADRONA     (527)
#define VEHICLE_FBITRUCK    (528)
#define VEHICLE_WILLARD     (529)
#define VEHICLE_FORKLIFT    (530)
#define VEHICLE_TRACTOR     (531)
#define VEHICLE_COMBINE     (532)
#define VEHICLE_FELTZER     (533)
#define VEHICLE_REMINGTN    (534)
#define VEHICLE_SLAMVAN     (535)
#define VEHICLE_BLADE       (536)
#define VEHICLE_FREIGHT     (537)
#define VEHICLE_STREAK      (538)
#define VEHICLE_VORTEX      (539)
#define VEHICLE_VINCENT     (540)
#define VEHICLE_BULLET      (541)
#define VEHICLE_CLOVER      (542)
#define VEHICLE_SADLER      (543)
#define VEHICLE_FIRELA      (544)
#define VEHICLE_HUSTLER     (545)
#define VEHICLE_INTRUDER    (546)
#define VEHICLE_PRIMO       (547)
#define VEHICLE_CARGOBOB    (548)
#define VEHICLE_TAMPA       (549)
#define VEHICLE_SUNRISE     (550)
#define VEHICLE_MERIT       (551)
#define VEHICLE_UTILITY     (552)
#define VEHICLE_NEVADA      (553)
#define VEHICLE_YOSEMITE    (554)
#define VEHICLE_WINDSOR     (555)
#define VEHICLE_MONSTERA    (556)
#define VEHICLE_MONSTERB    (557)
#define VEHICLE_URANUS      (558)
#define VEHICLE_JESTER      (559)
#define VEHICLE_SULTAN      (560)
#define VEHICLE_STRATUM     (561)
#define VEHICLE_ELEGY       (562)
#define VEHICLE_RAINDANC    (563)
#define VEHICLE_RCTIGER     (564)
#define VEHICLE_FLASH       (565)
#define VEHICLE_TAHOMA      (566)
#define VEHICLE_SAVANNA     (567)
#define VEHICLE_BANDITO     (568)
#define VEHICLE_FREIFLAT    (569)
#define VEHICLE_STREAKC     (570)
#define VEHICLE_KART        (571)
#define VEHICLE_MOWER       (572)
#define VEHICLE_DUNERIDE    (573)
#define VEHICLE_SWEEPER     (574)
#define VEHICLE_BROADWAY    (575)
#define VEHICLE_TORNADO     (576)
#define VEHICLE_AT400       (577)
#define VEHICLE_DFT30       (578)
#define VEHICLE_HUNTLEY     (579)
#define VEHICLE_STAFFORD    (580)
#define VEHICLE_BF400       (581)
#define VEHICLE_NEWSVAN     (582)
#define VEHICLE_TUG         (583)
#define VEHICLE_PETROTR     (584)
#define VEHICLE_EMPEROR     (585)
#define VEHICLE_WAYFARER    (586)
#define VEHICLE_EUROS       (587)
#define VEHICLE_HOTDOG      (588)
#define VEHICLE_CLUB        (589)
#define VEHICLE_FREIBOX     (590)
#define VEHICLE_ARTICT3     (591)
#define VEHICLE_ANDROM      (592)
#define VEHICLE_DODO        (593)
#define VEHICLE_RCCAM       (594)
#define VEHICLE_LAUNCH      (595)
#define VEHICLE_COPCARLA    (596)
#define VEHICLE_COPCARSF    (597)
#define VEHICLE_COPCARVG    (598)
#define VEHICLE_COPCARRU    (599)
#define VEHICLE_PICADOR     (600)
#define VEHICLE_SWATVAN     (601)
#define VEHICLE_ALPHA       (602)
#define VEHICLE_PHOENIX     (603)
#define VEHICLE_GLENSHIT    (604)
#define VEHICLE_SADLSHIT    (605)
#define VEHICLE_BAGBOXA     (606)
#define VEHICLE_BAGBOXB     (607)
#define VEHICLE_TUGSTAIR    (608)
#define VEHICLE_BOXBURG     (609)
#define VEHICLE_FARMTR1     (610)
#define VEHICLE_UTILTR1     (611)

#define SERVER_VARTYPE_NONE		(0)
#define SERVER_VARTYPE_INT 		(1)
#define SERVER_VARTYPE_STRING	(2)
#define SERVER_VARTYPE_FLOAT	(3)

native GetActorPoolSize();
native print(const output[]);
native printf(const output[], {Float,_}:...);
native format(buffer[], length, const format[], {Float,_}:...);
native SendClientMessage(playerid, color, const output[]);
native SendClientMessageToAll(color, const output[]);
native SendPlayerMessageToPlayer(playerid, senderid, const output[]);
native SendPlayerMessageToAll(senderid, const output[]);
native SendDeathMessage(killerid, killedid, weaponid);
native SendDeathMessageToPlayer(playerid, killer, killee, weapon);
native GameTextForAll(const output[], time, style);
native GameTextForPlayer(playerid, const output[], time, style);
native SetTimer(const funcname[], interval, repeat);
native SetTimerEx(const funcname[], interval, repeat, const params[], {Float,_}:...);
native KillTimer(timerid);
native GetTickCount();
native GetMaxPlayers();
native SetMaxRconLoginAttempt(max_attempt);
native CallRemoteFunction(const funcname[], const params[], {Float,_}:...);
native CallLocalFunction(const funcname[], const params[], {Float,_}:...);
native Float:VectorSize(Float:x, Float:y, Float:z);
native Float:asin(Float:value);
native Float:acos(Float:value);
native Float:atan(Float:value);
native Float:atan2(Float:y, Float:x);
native SHA256_PassHash(password[], salt[], ret_hash[], ret_hash_len);
native GetPlayerPoolSize();
native GetVehiclePoolSize();
native SetGameModeText(gamemode[]);
native SetTeamCount(count); // Not working
native AddPlayerClass(skinid, Float:x, Float:y, Float:z, Float:a, weap1, ammo1, weap2, ammo2, weap3, ammo3);
native AddPlayerClassEx(teamid, skinid, Float:x, Float:y, Float:z, Float:a, weap1, ammo1, weap2, ammo2, weap3, ammo3);
native AddStaticVehicle(modelid, Float:x, Float:y, Float:z, Float:a, color1, color2);
native AddStaticVehicleEx(modelid, Float:x, Float:y, Float:z, Float:a, color1, color2, respawndelay, addsiren=0);
native AddStaticPickup(modelid, typeid, Float:x, Float:y, Float:z, virtualworld);
native CreatePickup(modelid, typeid, Float:x, Float:y, Float:z, virtualworld);
native DestroyPickup(pickupid);
native DestroyAllPickups();
native IsValidPickup(pickupid);
native IsStaticPickup(pickupid);
native GetPickupPos(pickupid, &Float:x, &Float:y, &Float:z);
native GetPickupModel(pickupid);
native GetPickupType(pickupid);
native GetPickupCount();
native GetPickupPoolSize();
native GetPickupVirtualWorld(pickupid);
native ShowNameTags(show);
native ShowPlayerMarkers(show);
native DisableNameTagLOS();
native GameModeExit();
native SetWorldTime(hour);
native GetWorldTime();
native GetWeaponName(weaponid, const buffer[], length);
native FindWeaponID(const name[]);
native GetVehicleName(modelid, name[], len = sizeof(name));
native FindVehicleModel(const name[]);
native EnableTirePopping(enable);
native AllowInteriorWeapons(allow);
native SetWeather(weatherid);
native SetGravity(Float:gravity);
native Float:GetGravity();
native AllowAdminTeleport(allow);
native SetDeathDropAmount(amount);
native CreateExplosion(Float:x, Float:y, Float:z, type, Float:radius);
native EnableZoneNames(enable);
native SetDisabledWeapons(...);
native UsePlayerPedAnims();
native DisableInteriorEnterExits();
native DisableVehicleMarkers();
native SetNameTagDrawDistance(Float:distance);
native LimitGlobalChatRadius(Float:radius);
native LimitPlayerMarkerRadius(Float:marker_radius);
native IsPlayerAdmin(playerid);
native SetPlayerAdmin(playerid, toggle);
native Kick(playerid);
native Ban(playerid);
native BanEx(playerid, const reason[]);
native RemoveBan(ip[]);
native IsBanned(ip[]);
native SendRconCommand(const command[]);
native GetPlayerVersion(playerid, version[], len = sizeof(version));
native BlockIpAddress(ip_address[], timems);
native UnBlockIpAddress(ip_address[]);
native GetServerTickRate();
native GetServerVarAsString(const varname[], buffer[], length);
native GetServerVarAsInt(const varname[]);
native GetServerVarAsBool(const varname[]);
native GetConsoleVarAsString(const varname[], buffer[], length) = GetServerVarAsString;
native GetConsoleVarAsInt(const varname[]) = GetServerVarAsInt;
native GetConsoleVarAsBool(const varname[]) = GetServerVarAsBool;
native Menu:CreateMenu(const title[], columns, Float:x, Float:y, Float:col1width, Float:col2width = 0.0);
native DestroyMenu(Menu:menuid);
native AddMenuItem(Menu:menuid, column, const menutext[]);
native SetMenuColumnHeader(Menu:menuid, column, const columnheader[]);
native ShowMenuForPlayer(Menu:menuid, playerid);
native HideMenuForPlayer(Menu:menuid, playerid);
native IsValidMenu(Menu:menuid);
native DisableMenu(Menu:menuid);
native DisableMenuRow(Menu:menuid, row);
native Menu:GetPlayerMenu(playerid);
native Text:TextDrawCreate(Float:x, Float:y, text[]);
native TextDrawDestroy(Text:text);
native TextDrawLetterSize(Text:text, Float:x, Float:y);
native TextDrawTextSize(Text:text, Float:x, Float:y);
native TextDrawAlignment(Text:text, alignment);
native TextDrawColor(Text:text, color);
native TextDrawUseBox(Text:text, use);
native TextDrawBoxColor(Text:text, color);
native TextDrawSetShadow(Text:text, size);
native TextDrawSetOutline(Text:text, size);
native TextDrawBackgroundColor(Text:text, color);
native TextDrawFont(Text:text, font);
native TextDrawSetProportional(Text:text, set);
native TextDrawSetSelectable(Text:text, set);
native TextDrawShowForPlayer(playerid, Text:text);
native TextDrawHideForPlayer(playerid, Text:text);
native TextDrawShowForAll(Text:text);
native TextDrawHideForAll(Text:text);
native TextDrawSetString(Text:text, string[]);
native GangZoneCreate(Float:minx, Float:miny, Float:maxx, Float:maxy);
native GangZoneDestroy(gangzoneid);
native GangZoneShowForPlayer(playerid, gangzoneid, color);
native GangZoneShowForAll(gangzoneid, color);
native GangZoneHideForPlayer(playerid, gangzoneid);
native GangZoneHideForAll(gangzoneid);
native GangZoneFlashForPlayer(playerid, gangzoneid, color);
native GangZoneFlashForAll(gangzoneid, color);
native GangZoneStopFlashForPlayer(playerid, gangzoneid);
native GangZoneStopFlashForAll(gangzoneid);

native SetSVarInt(varname[], int_value);
native SetSVarString(varname[], string_value[]);
native SetSVarFloat(varname[], Float:float_value);
native GetSVarInt(varname[]);
native GetSVarString(varname[], string_return[], len);
native Float:GetSVarFloat(varname[]);
native DeleteSVar(varname[]);
native GetSVarType(varname[]);
native GetSVarNameAtIndex(index, ret_varname[], ret_len);
native GetSVarsUpperIndex();

native NetStats_GetConnectedTime(playerid);
native NetStats_MessagesReceived(playerid);
native NetStats_BytesReceived(playerid);
native NetStats_MessagesSent(playerid);
native NetStats_BytesSent(playerid);
native NetStats_MessagesRecvPerSecond(playerid);
native Float:NetStats_PacketLossPercent(playerid);
native NetStats_ConnectionStatus(playerid);
native NetStats_GetIpPort(playerid, ip_port[], ip_port_len);
native GetPlayerNetworkStats(playerid, retstr[], retstr_size = sizeof(retstr));
native GetNetworkStats(retstr[], retstr_size = sizeof(retstr));

forward OnGameModeInit();
forward OnGameModeExit();
forward OnFilterScriptInit();
forward OnFilterScriptExit();
forward OnPlayerConnect(playerid);
forward OnPlayerDisconnect(playerid, reason);
forward OnPlayerSpawn(playerid);
forward OnPlayerDeath(playerid, killerid, reason);
forward OnVehicleSpawn(vehicleid);
forward OnVehicleDeath(vehicleid, killerid);
forward OnPlayerBeginTyping(playerid);
forward OnPlayerEndTyping(playerid);
forward OnPlayerText(playerid, text[]);
forward OnPlayerCommandText(playerid, cmdtext[]);
forward OnPlayerRequestClass(playerid, classid);
forward OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
forward OnPlayerExitVehicle(playerid, vehicleid);
forward OnPlayerStateChange(playerid, newstate, oldstate);
forward OnPlayerEnterCheckpoint(playerid);
forward OnPlayerLeaveCheckpoint(playerid);
forward OnPlayerEnterRaceCheckpoint(playerid);
forward OnPlayerLeaveRaceCheckpoint(playerid);
forward OnRconCommand(cmd[]);
forward OnPlayerRequestSpawn(playerid);
forward OnObjectMoved(objectid);
forward OnPlayerObjectMoved(playerid, objectid);
forward OnPlayerPickUpPickup(playerid, pickupid);
forward OnVehicleMod(playerid, vehicleid, componentid);
forward OnEnterExitModShop(playerid, enterexit, interiorid);
forward OnVehiclePaintjob(playerid, vehicleid, paintjobid);
forward OnVehicleRespray(playerid, vehicleid, color1, color2);
forward OnPlayerSelectedMenuRow(playerid, row);
forward OnPlayerExitedMenu(playerid);
forward OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
forward OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
forward OnPlayerInfoChange(playerid);
forward OnRconLoginAttempt(ip[], password[], success);
forward OnPlayerUpdate(playerid);
forward OnPlayerClickMap(playerid, Float:fX, Float:fY, Float:fZ);
forward OnPlayerClickTextDraw(playerid, Text:clickedid);
forward OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid);
forward OnIncomingConnection(playerid, ip_address[], port);
forward OnTrailerUpdate(playerid, vehicleid);
forward OnPlayerStunt(playerid, vehicleid);
forward OnVehicleSirenStateChange(playerid, vehicleid, newstate);
forward OnVehicleDamageStatusUpdate(vehicleid, playerid);
forward OnActorStreamIn(actorid, forplayerid);
forward OnActorStreamOut(actorid, forplayerid);
forward OnPlayerGiveDamageActor(playerid, damaged_actorid, Float:amount, weaponid, bodypart);