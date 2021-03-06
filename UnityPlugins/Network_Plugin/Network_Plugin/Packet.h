#pragma once
#define MAX_PACKET_SIZE 6000
#define DEFAULT_DATA_SIZE 5000

#include <stdio.h>
#include <stdlib.h>   
#include <string>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

//enum PacketType {
//	// initialization connection
//	INIT = 0,
//	USER,
//	TYPE,
//	READY,
//	// single string
//	MESSAGE,
//	// game state
//	STATE,
//
//	// Entity Data
//	ENTITY,
//	// Damage dealt (int ID, bool Dir, int source, float damage)
//	DAMAGE,
//
//	// FPS weapon switch
//	WEAPON,
//
//	//RTS Managed Data
//	// entity built
//	BUILD,
//	// entity killed
//	DEATH
//};
//
//enum PlayerType {
//	OTHER = 0,
//	RTS,
//	FPS
//};
//
//enum PlayerMask
//{
//	SERVER = 1 << 0,
//	CLIENT1 = 1 << 1,
//	CLIENT2 = 1 << 2,
//	CLIENT3 = 1 << 3,
//	CLIENT4 = 1 << 4
//};
//
//enum GameState
//{
//	LOBBY = 0,
//	TIMER,
//	LOAD,
//	GAME,
//	ENDGAME
//};


//struct Packet {
//
//	unsigned int packet_type;
//	int sender = 0;
//	char data[DEFAULT_DATA_SIZE];
//	unsigned int id = 0;
//
//	void serialize(char* data) {
//		memcpy(data, this, sizeof(Packet));
//	}
//
//	void deserialize(char* data) {
//		memcpy(this, data, sizeof(Packet));
//	}
//};
//
//NETWORK_H struct packet_init {
//	int index;
//};
//
//NETWORK_H struct packet_join {
//	PlayerType type;		// 0 = rts, 1 = fps
//	int playerID;
//};
//
//NETWORK_H struct packet_msg {
//	char* message;
//};
//
//NETWORK_H struct entity {
//	float posX;
//	float posY;
//	float posZ;
//	float rotX;
//	float rotY;
//	float rotZ;
//	int state;
//};
//
//NETWORK_H struct packet_entity {
//	entity entities[500];
//};
//
//NETWORK_H struct packet_weapon {
//	int weapon;
//};
//
//NETWORK_H struct packet_damage {
//	int playerID;
//	bool dir;
//	int entity;
//	float damage;
//};
//
//NETWORK_H struct packet_build {
//	int id;
//	int type;
//	float posX;
//	float posY;
//	float posZ;
//};
//
//NETWORK_H struct packet_kill {
//	int id;
//};
//
//NETWORK_H struct packet_state {
//	int state;
//};