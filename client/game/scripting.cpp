//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: scripting.cpp,v 1.6 2006/03/20 17:44:20 kyeman Exp $
//
//----------------------------------------------------------

#include "../main.h"

GAME_SCRIPT_THREAD* gst;
static FARPROC ProcessOneCommand = (FARPROC)0x469EB0;
static BYTE ScriptBuf[255];
static DWORD* pdwParamVars[18];

DWORD dwScmOpcodeDebug=0;
bool bScmLocalDebug=false;
//static bool bExceptionDisplayed=false;

int __declspec(naked) ExecuteScriptBuf()
{
	__asm
	{
		mov bScmLocalDebug, 1;

		mov eax, OFFSET ScriptBuf	// Move our script buffer base into eax.

		mov ecx, gst				// Move GAME_SCRIPT_THREAD structure into ecx.
		mov [ecx+0x14], eax			// Move eax into the gst->dwScriptIP.

		call ProcessOneCommand		// Call the game's script opcode processor.

		mov ecx, gst				// Move game script thread into ecx again.
		mov eax, [ecx+0xC5]			// Move the dwIfFlag into eax (return value).

		mov bScmLocalDebug, 0;

		ret							// return.
	}
}


int ScriptCommand(const SCRIPT_COMMAND* pScriptCommand, ...)
{
	va_list ap;
	const char* p = pScriptCommand->Params;	// Get parameter string.
	va_start(ap, pScriptCommand);			// Initialize varargs.
	memcpy(&ScriptBuf, &pScriptCommand->OpCode, 2);	// Copy opcode to script buf.
	int buf_pos = 2;	// Position in buffer after opcode.
	WORD var_pos = 0;	// Init var_pos.
	
	// (aru) Reset all vars before proceeding (To fix some nasty 0x00465CB4 crashes)
	// May cause problems with functions that use local vars returned from previous calls
	
	for(int i=0; i<18; i++)
		gst->dwLocalVar[i] = 0;

	dwScmOpcodeDebug = pScriptCommand->OpCode;

	while(*p)			// While we've not ran out of parameters...
	{
		switch(*p)		// Switch current parameter.
		{
			case 'i':	// If integer...
			{
				int i = va_arg(ap, int);			// Grab an int off the stack.
				ScriptBuf[buf_pos] = 0x01;			// Variable data type = 0x01 (4b int).
				buf_pos++;;							// Increment buffer position.
				memcpy(&ScriptBuf[buf_pos], &i, 4);	// Insert the int.
				buf_pos += 4;						// Increment buffer by 4b.
				break;
			}
			case 'f':	// If float...
			{
				/*	Take note, MSVC++ puts a double on the stack
					even if you typecase as a float. <3 Microsoft.	*/
				float f = (float)va_arg(ap, double);	// Get float off the stack.
				ScriptBuf[buf_pos] = 0x06;				// Variable data type = 0x06 (float).
				buf_pos++;								// Increment buffer position.
				memcpy(&ScriptBuf[buf_pos], &f, 4);		// Copy float into script buf.
				buf_pos += 4;							// Increment buffer by 4b.
				break;
			}
			case 'v':	// If variable...
			{
				DWORD* v = va_arg(ap, DWORD*);				// Get the pointer to the passed variable.
				ScriptBuf[buf_pos] = 0x03;					// Variable data type = 0x03 (Local Var).
				buf_pos++;									// Increment buffer position.
				pdwParamVars[var_pos] = v;					// Save pointer to passed variable.
				gst->dwLocalVar[var_pos] = *v;				// Put value of passed variable into local.
				memcpy(&ScriptBuf[buf_pos], &var_pos, 2);	// Copy the offset into the script.
				buf_pos += 2;								// Increment buffer by 2b.
				var_pos++;									// Increment the variable position.
				break;
			}
			case 's':	// If string... Updated 13th Jan 06.. (kyeman) SA string support
			{
				char* sz = va_arg(ap, char*);
				size_t uiLen = strlen(sz);
				ScriptBuf[buf_pos] = 0x0E;
				buf_pos++;
				ScriptBuf[buf_pos] = (BYTE)uiLen;
				buf_pos++;
				memcpy(&ScriptBuf[buf_pos],sz, uiLen);
				buf_pos += uiLen;
				break;
			}
			case 'z':	// If the params need zero-terminating...
			{
				ScriptBuf[buf_pos] = 0x00;			
				buf_pos++;
				break;
			}
			default:
			{
				return 0;
			}
		}
		++p;		// Next parameter
	}
	va_end(ap);		// End varargs.

	// Execute script stub.
	int result =0;
	
	__try {

		// Enable SCM
		*(PBYTE)0x469EF5 = 0xFF;
		*(PBYTE)0x469EF6 = 0xD2;

		result = ExecuteScriptBuf();
		if (var_pos)	// if we've used a variable...
		{
			for (int i=0; i<var_pos; i++)		// For every passed variable...
			{
				*pdwParamVars[i] = gst->dwLocalVar[i];	// Retrieve variable from local var.
			}
		}

		// Disable SCM
		*(PBYTE)0x469EF5 = 0x8B;
		*(PBYTE)0x469EF6 = 0xD0;

	} __except (exc_filter(GetExceptionCode(), GetExceptionInformation(), "opcode")) {
		return 0;
	}


	return result;
}


void InitScripting()
{
	gst = new GAME_SCRIPT_THREAD;
	ZeroMemory(gst, sizeof(GAME_SCRIPT_THREAD));
}
