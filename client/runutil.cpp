//----------------------------------------------------
//
//	Utility runtime procedures
//	(c) 2005 Kye Bitossi
//
// Version: $Id: runutil.cpp,v 1.3 2006/03/20 17:44:19 kyeman Exp $
//
//----------------------------------------------------

#include "main.h"

#include <stdio.h>
#include <string.h>

//----------------------------------------------------

int FormatChatBubbleText(char* szText, int iWidth, int iMaxWord)
{
	int s, i, r, w, c;
	char buf[512];

	r = strlen(szText);
	w = iWidth;
	i = 0;
	c = 1;

	memset(buf, 0, sizeof(buf));
	
	//if (strlen(szText) <= 512)
	if (r <= (int)sizeof(buf))
	{
		while (r > iWidth)
		{
			s = iWidth;

			while (s != 0 && szText[i + s] != ' ')
				--s;

			if (w - i - s <= iMaxWord)
			{
				i += s;
				szText[i] = '\n';
				r -= s;
				w = i + iWidth;
			}
			else
			{
				strcpy_s(buf, szText + w);
				szText[w + 1] = 0;
				szText[w] = '\n';
				memcpy(szText + strlen(szText), buf, strlen(buf) + 1);
				i = w + 1;
				r -= iWidth;
				w += 1 + iWidth;
			}
			c++;
		}
		return c;
	}
	return -1;
}

//----------------------------------------------------

void Util_UrlUnencode(char *enc)
{
	char *write_pos = enc;

	while(*enc)
	{
		if(*enc=='%')
		{
			*write_pos = (*++enc>'/'&&*enc<':')?((*enc-('0'))<<4):((*enc-('7'))<<4);
			*write_pos |= (*++enc>'/'&&*enc<':')?(*enc-'0'):(*enc-'7');
		}
		else if (*enc=='+')
			*write_pos= ' ';
		else
			*write_pos= *enc;

		write_pos++; enc++;
	}
	*write_pos='\0';
}

//----------------------------------------------------

char Util_toupper(char c) {return ((c>(char)0x60) && (c<(char)0x7b))? c-0x20:c;}

//----------------------------------------------------

char *Util_stristr(const char *String, const char *Pattern)
{
	char *pptri, *sptri, *starti;

      for (starti=(char *)String; *starti != '\0'; starti++)
      {

            /* find start of pattern in string */
            for (;((*starti!='\0') && (Util_toupper(*starti) != Util_toupper(*Pattern))); starti++);

            pptri = (char *)Pattern;
            sptri = (char *)starti;

            while (Util_toupper(*sptri) == Util_toupper(*pptri))
            {
                  sptri++;
                  pptri++;

                  /* if end of pattern then pattern was found */
                  if ('\0' == *pptri)
                        return (starti);
            }

      }

      return(0);
}

//----------------------------------------------------

void Util_strupr(char *string)
{
	char *p = string;

	while(*p) {
		*p = Util_toupper(*p);
		p++;
	}
}

//----------------------------------------------------

int Util_wildcmp(char *wild, char *string)
{
	char *cp, *mp;
	
	while((*string) && (*wild != '*'))
	{
		if((*wild != *string) && (*wild != '?'))
		{
			return 0;
		}
		wild++;
		string++;
	}
		
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
		
	while (*wild == '*')
	{
		wild++;
	}

	return !*wild;
}

//----------------------------------------------------

int Util_strnicmp(const char *s1, const char *s2, size_t n)
{

  if (n == 0) return 0;

  do
  {
    if (Util_toupper((unsigned char)*s1) != Util_toupper((unsigned char)*s2++))
      return (int)Util_toupper((unsigned char)*s1) - (int)Util_toupper((unsigned char)*--s2);
    if (*s1++ == 0)
      break;

  } while (--n != 0);

  return 0;
}

//----------------------------------------------------

char *Util_strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

//----------------------------------------------------

char * Util_itoa(int v, char *s, int r)
{
	int i,neg = 0;
	char *p = s;
	char *q = s;

	if (r < 0 || r > 35) {
		*s = 0;
		return (s);
		}
	if (r == 0) r = 10;
	if (v == 0) {
		*p++ = '0';
		*p = 0;
		return (s);
		}
	if (v < 0) {
		neg = 1;
		v = -v;
		}
	while (v > 0) {
		i = v % r;
		if (i > 9) i += 7;
		*p++ = '0' + i;
		v /= r;
		}
	if (neg) *p++ = '-';
	*p-- = 0;
	q = s;
	while (p > q) {
		i = *q;
		*q++ = *p;
		*p-- = i;
		}
	return (s);
}

//----------------------------------------------------

char * Base64Encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void Util_Base64Encode( char *cpInput, char *cpOutput )
{
int nIdx[ 4 ];  
while ( '\0' != *cpInput )
{
  nIdx[0] = ((*cpInput) & 0xFC)>>2;
  nIdx[1] = ((*cpInput) & 0x03)<<4;
  cpInput++;
  if ( '\0' != *cpInput )
  {
    nIdx[1] |= ((*cpInput) & 0xF0)>>4;
    nIdx[2]  = ((*cpInput) & 0x0F)<<2;
    cpInput++;
    if ( '\0' != (*cpInput) )
    {
      nIdx[2] |= ((*cpInput) & 0xC0) >> 6;
      nIdx[3]  = (*cpInput) & 0x3F;
      cpInput++;
    }
    else
      nIdx[3] = 64;
  }
  else
  {
    nIdx[2] = 64;
    nIdx[3] = 64;
  }

  *(cpOutput+0) = *(Base64Encoding + nIdx[0]);
  *(cpOutput+1) = *(Base64Encoding + nIdx[1]);
  *(cpOutput+2) = *(Base64Encoding + nIdx[2]);
  *(cpOutput+3) = *(Base64Encoding + nIdx[3]);
  cpOutput += 4;
}

*cpOutput = '\0';

return;
}

//----------------------------------------------------
// Simple rotate right 3 character encoding for
// hiding strings in the exe.

void K_EncodeString(char *szInput, char *szOutput)
{
	char b;

	while(*szInput) {
		b = *szInput;
		_asm mov bl, b
		_asm ror bl, 3
		_asm mov b, bl
		*szOutput = b;
		szInput++;
		szOutput++;
	}
	*szOutput = 0;
}

//----------------------------------------------------

char * K_DecodeString(unsigned char *szInput)
{
	char b;
	char *st = (char *)szInput;
    
	while(*szInput) {
		b = *szInput;
		_asm mov bl, b
		_asm rol bl, 3
		_asm mov b, bl
		*szInput = b;
		szInput++;
	}

	return st;
}

unsigned long Util_GetTime()
{
	static bool bInited = false;
	static LARGE_INTEGER fli;
	LARGE_INTEGER cli;
	if (!bInited) {
		QueryPerformanceFrequency(&fli);
		bInited = true;
	}
	QueryPerformanceCounter(&cli);
	return (unsigned long)(cli.QuadPart * 1000 / fli.QuadPart);
}

const char* GetWeaponName(int iWeaponID)
{
	switch (iWeaponID) {
	case WEAPON_FIST:
		return "Fist";
	case WEAPON_BRASSKNUCKLE:
		return "Brass Knuckles";
	case WEAPON_GOLFCLUB:
		return "Golf Club";
	case WEAPON_NITESTICK:
		return "Nite Stick";
	case WEAPON_KNIFE:
		return "Knife";
	case WEAPON_BAT:
		return "Baseball Bat";
	case WEAPON_SHOVEL:
		return "Shovel";
	case WEAPON_POOLSTICK:
		return "Pool Cue";
	case WEAPON_KATANA:
		return "Katana";
	case WEAPON_CHAINSAW:
		return "Chainsaw";
	case WEAPON_DILDO:
		return "Dildo";
	case WEAPON_DILDO2:
		return "Dildo";
	case WEAPON_VIBRATOR:
		return "Vibrator";
	case WEAPON_VIBRATOR2:
		return "Vibrator";
	case WEAPON_FLOWER:
		return "Flowers";
	case WEAPON_CANE:
		return "Cane";
	case WEAPON_GRENADE:
		return "Grenade";
	case WEAPON_TEARGAS:
		return "Teargas";
	case WEAPON_MOLTOV:
		return "Molotov Cocktail";
	case WEAPON_COLT45:
		return "Colt 45";
	case WEAPON_SILENCED:
		return "Silenced Pistol";
	case WEAPON_DEAGLE:
		return "Desert Eagle";
	case WEAPON_SHOTGUN:
		return "Shotgun";
	case WEAPON_SAWEDOFF:
		return "Sawn-off Shotgun";
	case WEAPON_SHOTGSPA:
		return "Combat Shotgun";
	case WEAPON_UZI:
		return "UZI";
	case WEAPON_MP5:
		return "MP5";
	case WEAPON_AK47:
		return "AK47";
	case WEAPON_M4:
		return "M4";
	case WEAPON_TEC9:
		return "TEC9";
	case WEAPON_RIFLE:
		return "Rifle";
	case WEAPON_SNIPER:
		return "Sniper Rifle";
	case WEAPON_ROCKETLAUNCHER:
		return "Rocket Launcher";
	case WEAPON_HEATSEEKER:
		return "Heat Seaker";
	case WEAPON_FLAMETHROWER:
		return "Flamethrower";
	case WEAPON_MINIGUN:
		return "Minigun";
	case WEAPON_SATCHEL:
		return "Satchel Explosives";
	case WEAPON_BOMB:
		return "Bomb";
	case WEAPON_SPRAYCAN:
		return "Spray Can";
	case WEAPON_FIREEXTINGUISHER:
		return "Fire Extinguisher";
	case WEAPON_CAMERA:
		return "Camera";
	case WEAPON_NIGHT_VIS_GOGGLES:
		return "Night Vision Goggles";
	case WEAPON_THERMAL_GOGGLES:
		return "Thermal Goggles";
	case WEAPON_PARACHUTE:
		return "Parachute";
	case WEAPON_VEHICLE:
		return "Vehicle";
	case WEAPON_DROWN:
		return "Drowned";
	case WEAPON_COLLISION:
		return "Splat";
	}
	return "";
}

//----------------------------------------------------

bool DirExists(LPCSTR szPath)
{
	DWORD dwAttr;

	//struct _stat64i32 status;
	//return _stat(szPath, &status) == 0;

	dwAttr = GetFileAttributes(szPath);

	return dwAttr != INVALID_FILE_ATTRIBUTES &&
		dwAttr & FILE_ATTRIBUTE_DIRECTORY;
}

//----------------------------------------------------

bool IsHexChar(char c)
{
	return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

//----------------------------------------------------

bool IsHexCharW(wchar_t c)
{
	return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

//----------------------------------------------------

unsigned long GetColorFromStringEmbed(char* szString)
{
	char szHex[7] = { 0 }; // [17];

	if (*szString &&
		*szString == '{' &&
		*(szString + 1) &&
		IsHexChar(*(szString + 1)) &&
		*(szString + 2) &&
		IsHexChar(*(szString + 2)) &&
		*(szString + 3) &&
		IsHexChar(*(szString + 3)) &&
		*(szString + 4) &&
		IsHexChar(*(szString + 4)) &&
		*(szString + 5) &&
		IsHexChar(*(szString + 5)) &&
		*(szString + 6) &&
		IsHexChar(*(szString + 6)) &&
		*(szString + 7) &&
		*(szString + 7) == '}')
	{
		//memset(szHex, 0, sizeof(szHex));
		strncpy_s(szHex, szString + 1, 6);
		return strtoul(szHex, 0, 16);
	}
	return -1;
}

//----------------------------------------------------

unsigned long GetColorFromStringEmbedW(wchar_t* wszString)
{
	wchar_t wszHex[7] = { 0 }; // [17];

	if (*wszString &&
		*wszString == '{' &&
		*(wszString + 1) &&
		IsHexCharW(*(wszString + 1)) &&
		*(wszString + 2) &&
		IsHexCharW(*(wszString + 2)) &&
		*(wszString + 3) &&
		IsHexCharW(*(wszString + 3)) &&
		*(wszString + 4) &&
		IsHexCharW(*(wszString + 4)) &&
		*(wszString + 5) &&
		IsHexCharW(*(wszString + 5)) &&
		*(wszString + 6) &&
		IsHexCharW(*(wszString + 6)) &&
		*(wszString + 7) &&
		*(wszString + 7) == '}')
	{
		//memset(wszHex, 0, sizeof(wszHex));
		wcsncpy_s(wszHex, wszString + 1, 6);
		return wcstoul(wszHex, 0, 16); // wcstoxl ?!
	}
	return -1;
}

//----------------------------------------------------

void RemoveColorEmbedsFromString(char* szString)
{
	char* szCurrent, * szNext;

	szCurrent = szString;
	szNext = szString + 8;
	while (*szCurrent)
	{
		if (GetColorFromStringEmbed(szCurrent) == (unsigned long)-1)
		{
			szCurrent++;
			szNext++;
		}
		else
			strcpy_s(szCurrent, RSIZE_MAX, szNext);
	}
	*szCurrent = '\0';
}

//----------------------------------------------------

int ConvertMultiToWideString(LPCSTR szSource, LPWSTR szDest, int iLen)
{
	size_t nSourceLen;
	int iResult;

	SecureZeroMemory(szDest, sizeof(WCHAR) * iLen);

	nSourceLen = strlen(szSource);

	//iResult = MultiByteToWideChar(CP_ACP, 0, szSource, strlen(nSrcLen), NULL, 0);
	iResult = MultiByteToWideChar(CP_ACP, 0, szSource, nSourceLen, NULL, 0);

	if (iResult < iLen)
		//iResult = MultiByteToWideChar(CP_ACP, 0, szSource, strlen(szSource), szDest, iResult);
		iResult = MultiByteToWideChar(CP_ACP, 0, szSource, nSourceLen, szDest, iResult);

	return iResult;
}

//----------------------------------------------------

/*
	Index	Location	Text								Default key
	-----------------------------------------------------------------------
	0.		0xB703BC	PED_FIREWEAPON						"LMB"
	1.		0xB703E4	PED_FIREWEAPON_ALT					"\"
	2.		0xB7040C	PED_CYCLE_WEAPON_RIGHT				"MS WHEEL DN"
	3.		0xB70434	PED_CYCLE_WEAPON_LEFT				"MS WHEEL UP"
	4.		0xB7045C	GO_FORWARD							"UP"
	5.		0xB70484	GO_BACK								"DOWN"
	6.		0xB704AC	GO_LEFT								"LEFT"
	7.		0xB704D4	GO_RIGHT							"RIGHT"
	8.		0xB704FC	PED_SNIPER_ZOOM_IN					"MS WHEEL UP"
	9.		0xB70524	PED_SNIPER_ZOOM_OUT					"MS WHEEL DN"
	10.		0xB7054C	VEHICLE_ENTER_EXIT					"RETURN"
	11.		0xB70574	CAMERA_CHANGE_VIEW_ALL_SITUATIONS	"V"
	12.		0xB7059C	PED_JUMPING							"LSHIFT"
	13.		0xB705C4	PED_SPRINT							"SPACE"
	14.		0xB705EC	PED_LOOKBEHIND						"MMB"
	15.		0xB70614	PED_DUCK							"C"
	16.		0xB7063C	PED_ANSWER_PHONE					"TAB"
	17.		0xB70664	SNEAK_ABOUT							"LALT"
	18.		0xB7068C	VEHICLE_FIREWEAPON					"LMB"
	19.		0xB706B4	VEHICLE_FIREWEAPON_ALRT				"LCTRL"
	20.		0xB706DC	VEHICLE_STEERLEFT					"A"
	21.		0xB70704	VEHICLE_STEERRIGHT					"D"
	22.		0xB7072C	VEHICLE_STEERUP						"UP"
	23.		0xB70754	VEHICLE_STEERDOWN					"DOWN"
	24.		0xB7077C	VEHICLE_ACCELERATE					"W"
	25.		0xB707A4	VEHICLE_BRAKE						"S"
	26.		0xB707CC	VEHICLE_RADIO_STATION_UP			"MS WHEEL UP"
	27.		0xB707F4	VEHICLE_RADIO_STATION_DOWN			"MS WHEEL DN"
	28.		0xB7081C										"F5"
	29.		0xB70844	VEHICLE_HORN						"CAPSLOCK"
	30.		0xB7086C	TOGGLE_SUBMISSIONS					"2"
	31.		0xB70894	VEHICLE_HANDBRAKE					"SPACE"
	32.		0xB708BC	PED_1RST_PERSON_LOOK_LEFT			"NUM4"
	33.		0xB708E4	PED_1RST_PERSON_LOOK_RIGHT			"NUM6"
	34.		0xB7090C	VEHICLE_LOOKLEFT					"Q"
	35.		0xB70934	VEHICLE_LOOKRIGHT					"E"
	36.		0xB7095C	VEHICLE_LOOKBEHIND					"MMB"
	37.		0xB70984	VEHICLE_MOUSELOOK					"RMB"
	38.		0xB709AC	VEHICLE_TURRETLEFT					"NUM4"
	39.		0xB709D4	VEHICLE_TURRETRIGHT					"NUM6"
	40.		0xB709FC	VEHICLE_TURRETUP					"NUM2"
	41.		0xB70A24	VEHICLE_TURRETDOWN					"NUM8"
	42.		0xB70A4C	PED_CYCLE_TARGET_LEFT				"["
	43.		0xB70A74	PED_CYCLE_TARGET_RIGHT				"]"
	44.		0xB70A9C	PED_CENTER_CAMERA_BEHIND_PLAYER		"#"
	45.		0xB70AC4	PED_LOCK_TARGET						"RMB"
	46.		0xB70AEC	NETWORK_TALK						""
	47.		0xB70B14	CONVERSATION_YES					"Y"
	48.		0xB70B3C	CONVERSATION_NO						"N"
	49.		0xB70B64	GROUP_CONTROL_FWD					"G"
	50.		0xB70B8C	GROUP_CONTROL_BWD					"H"
	51.		0xB70BB4	PED_1RST_PERSON_LOOK_UP				"NUM2"
	52.		0xB70BDC	PED_1RST_PERSON_LOOK_DOWN			"NUM8"
	53.		0xB70C04										""
	54.		0xB70C2C	TOGGLE_DPAD							""
	55.		0xB70C54	SWITCH_DEBUG_CAM_ON					""
	56.		0xB70C7C	TAKE_SCREEN_SHOT					""
	57.		0xB70CA4	SHOW_MOUSE_POINTER_TOGGLE			""
	58.		0xB70CCC										""
*/

void FormatGameKeysInString(PCHAR buf)
{
	char output[2049];
	char gxt[50];

	if (!buf) return;

	typedef struct
	{
		char szNames[59][40];
	} ACTIONS;

	// byte_B703BC[59][40]; ~ size=2360
	ACTIONS* pActions = (ACTIONS*)0xB703BC;

	size_t buflen = strlen(buf);
	size_t k = 0;
	memset(gxt, 0, sizeof(gxt));

	for (size_t i = 0; i < buflen; i++)
	{
		if ((i + 3) < buflen &&
			buf[i] == '~' &&
			buf[i + 1] == 'k' &&
			buf[i + 2] == '~' &&
			buf[i + 3] == '~')
		{
			bool found = false;
			i += 4;
			for (size_t j = 0; j < 53 && !found; j++)
			{
				size_t keylen = strlen(pActions->szNames[j]);

				if (keylen != 0 &&
					strncmp(&buf[i], pActions->szNames[j], keylen) == 0 &&
					buf[i + keylen] == '~')
				{
					found = true;

					if (j != 46) // NETWORK_TALK
					{
						_asm push 50
						_asm lea edx, gxt
						_asm push edx
						_asm push j
						_asm mov ecx, 0xB70198
						_asm mov edx, 0x5303D0
						_asm call edx
					}

					i += keylen;

					size_t gxtlen = strlen(gxt);
					if (gxtlen)
					{
						for (size_t l = 0; l < gxtlen; l++)
						{
							if (k >= sizeof(output) - 1)
								break;
							output[k] = gxt[l];
							k++;
						}
						gxt[0] = '\0';
					}
				}
			}
			if (!found)
			{
				output[k++] = '~';
				output[k++] = 'k';
				output[k++] = '~';
				output[k++] = '~';
				i--;
			}
		}
		else
		{
			if (k >= sizeof(output) - 1)
				break;

			output[k] = buf[i];
			k++;
		}
	}
	output[k] = '\0';

	strcpy_s(buf, 2048, output);
}

//----------------------------------------------------

// This only supports the maximum 400 characters
DWORD FormatGameTextKey(PCHAR szBuf, DWORD dwMaxLen)
{
	if (szBuf && strlen(szBuf) < 400 && strstr(szBuf, "~k~"))
	{
		char tmp_buf[2048];
		memset(tmp_buf, 0, sizeof(tmp_buf));
		strcpy_s(tmp_buf, szBuf);

		_asm lea edx, tmp_buf
		_asm push edx
		_asm mov edx, 0x69E160
		_asm call edx
		_asm pop edx

		DWORD dwLen = strlen(tmp_buf);
		if (dwLen <= dwMaxLen)
		{
			strcpy_s(szBuf, dwMaxLen, tmp_buf);
			return dwLen; //strlen(tmp_buf);
		}
	}
	return 0;
}

//----------------------------------------------------

