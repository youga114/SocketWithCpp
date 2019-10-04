#pragma once
#include <WinSock2.h>
#include "Common.h"

class Player
{
public:
	Player();
	~Player();

	uint16 UserID;
	int16 X;
	int16 Y;
	SOCKET MySocket;
};

