#pragma once
#include "Common.h"
#include "Player.h"

class PacketMaker
{
public:
	PacketMaker();
	~PacketMaker();

	uint16 PacketSize;

	void MakeEnterPacket(const Player& Data);
	void MakeExitPacket(const Player& Data);
	void MakeMovePacket(const Player& Data);

	void ExtractPacket(MSGCode Code, char* NewPacket, Player* OutPlayer);

	char Packet[255];
};

