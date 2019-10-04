#pragma once

typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;

enum class MSGCode
{
	Enter = 0,
	Exit = 1,
	Move = 2,
	Bye = 3,
};