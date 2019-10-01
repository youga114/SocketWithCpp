#include "PacketMaker.h"
#include <stdlib.h>
#include <string.h>

PacketMaker::PacketMaker()
{
}


PacketMaker::~PacketMaker()
{
}

void PacketMaker::MakeEnterPacket(const Player& Data)
{
	PacketSize = 5;

	memset(Packet, 0, sizeof(Packet));

	uint16 Code = (uint16)MSGCode::Enter;
	memcpy(Packet, &Code, sizeof(uint16));
	memcpy(Packet + 2, &PacketSize, 1);
	memcpy(Packet + 3, &Data.UserID, sizeof(uint16));
}

void PacketMaker::MakeExitPacket(const Player& Data)
{
	PacketSize = 5;

	memset(Packet, 0, sizeof(Packet));

	uint16 Code = (uint16)MSGCode::Exit;
	memcpy(Packet, &Code, sizeof(uint16));
	memcpy(Packet + 2, &PacketSize, 1);
	memcpy(Packet + 2, &Data.UserID, sizeof(uint16));
}

void PacketMaker::MakeMovePacket(const Player& Data)
{
	PacketSize = 9;

	memset(Packet, 0, sizeof(Packet));

	uint16 Code = (uint16)MSGCode::Move;
	memcpy(Packet, &Code, sizeof(uint16));
	memcpy(Packet + 2, &PacketSize, 1);
	memcpy(Packet + 3, &Data.UserID, sizeof(uint16));
	memcpy(Packet + 5, &Data.X, sizeof(uint16));
	memcpy(Packet + 7, &Data.Y, sizeof(uint16));
}

void PacketMaker::ExtractPacket(MSGCode Code, char* NewPacket, Player* OutPlayer)
{
	switch (Code)
	{
		case MSGCode::Enter:
		{
			memcpy(&OutPlayer->UserID, NewPacket, 2);
		}
		break;

		case MSGCode::Exit:
		{
			memcpy(&OutPlayer->UserID, NewPacket, 2);
		}
		break;

		case MSGCode::Move:
		{
			memcpy(&OutPlayer->UserID, NewPacket, 2);
			memcpy(&OutPlayer->X, NewPacket + 2, 2);
			memcpy(&OutPlayer->Y, NewPacket + 4, 2);
		}
		break;
	}
}
