
#if defined _objects_included
	#endinput
#endif
#define _objects_included

native CreateObject(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz);
native AttachObjectToPlayer(objectid, playerid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz);
native SetObjectPos(objectid, Float:x, Float:y, Float:z);
native GetObjectPos(objectid, &Float:x, &Float:y, &Float:z);
native SetObjectRot(objectid, Float:rx, Float:ry, Float:rz);
native GetObjectRot(objectid, &Float:rx, &Float:ry, &Float:rz);
native GetObjectModel(objectid);
native IsValidObject(objectid);
native DestroyObject(objectid);
native MoveObject(objectid, Float:x, Float:y, Float:z, Float:speed);
native StopObject(objectid);
native IsObjectMoving(objectid);
native SetObjectScale(objectid, Float:scale);

native CreatePlayerObject(playerid, modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz);
native SetPlayerObjectPos(playerid, playerobjectid, Float:x, Float:y, Float:z);
native GetPlayerObjectPos(playerid, playerobjectid, &Float:x, &Float:y, &Float:z);
native SetPlayerObjectRot(playerid, playerobjectid, Float:rx, Float:ry, Float:rz);
native GetPlayerObjectRot(playerid, playerobjectid, &Float:x, &Float:y, &Float:z);
native IsValidPlayerObject(playerid, playerobjectid);
native DestroyPlayerObject(playerid, playerobjectid);
native MovePlayerObject(playerid, playerobjectid, Float:x, Float:y, Float:z, Float:speed);
native StopPlayerObject(playerid, playerobjectid);
native GetPlayerObjectModel(playerid, objectid);
native IsPlayerObjectMoving(playerid, playerobjectid);
native AttachPlayerObjectToPlayer(playerid, playerobjectid, toplayerid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz);
