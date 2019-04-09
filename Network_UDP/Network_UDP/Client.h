#pragma once
#include "ExchangeContent.h"

#define MYSHARED_API __declspec(dllexport)

class Client
{
private:
	SOCKET m_handle;
	sockaddr_in m_serverHint;
	
	char m_buffer[254];
	State m_state;
	int m_id;

public:
	Client();
	~Client();

	void Close();

	bool Start(unsigned int p_port = 9842);
	bool AttemptToConnect();

	int Receive();
	int Received(char* buffer, int size);
	int	Send(char* data, int size, PacketType type = PacketType::DATA);
	int GetID();
	
	uint8_t GetState();
	char* GetBuffer();
private:
};

extern "C"
{
	MYSHARED_API Client* CreateClient();
	MYSHARED_API bool	 CStart(Client* p_obj,unsigned int p_port);
	MYSHARED_API bool	 CAttemptToConnect(Client* p_obj);
	MYSHARED_API int	 CReceive(Client* p_obj);
	MYSHARED_API int	 CReceived(Client* p_obj, char* p_buffer,int size);
	MYSHARED_API int	 CGetID(Client* p_obj);
	MYSHARED_API int	 CSend(Client* p_obj, char* p_data, int size);
	MYSHARED_API void	 CClose(Client* p_obj);
	MYSHARED_API uint8_t GetState(Client* p_obj);
	MYSHARED_API char*	 CGetBuffer(Client* p_obj);

}