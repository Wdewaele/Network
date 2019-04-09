#pragma once

#include "ExchangeContent.h"

#define MYSHARED_API __declspec(dllexport)

//struct packet

struct ClientInfo
{
	sockaddr_in m_clientAddress;
	int m_id;

	bool operator==(ClientInfo p_other)
	{
		return m_id == p_other.m_id;
	}
};

class Server
{
private:
	SOCKET m_handle;
	sockaddr_in m_receivedAddress;

	std::array<ClientInfo,4> m_clients;
	std::array<bool, 4> m_isConnected;

	char m_buffer[254]{};
	int m_bufferSize;
public:
	Server();
	~Server();

	void Run();
	void Close();

	int Send(sockaddr_in& p_client);
	int Send(char* p_buffer, int size);
	int Receive(sockaddr_in& p_client);
	int Listen();

	bool Start(unsigned int p_port = 9842);
	ClientInfo GetClientInfo(int index);

private:
	void CheckFirstConnection(sockaddr_in& p_client);
	int FindClient(sockaddr_in& p_client);
	int FindOpenSlot();

};

bool operator==(const sockaddr_in& a, const sockaddr_in& b)
{
	return a.sin_port == b.sin_port && a.sin_addr.s_addr == b.sin_addr.s_addr;
}
bool operator!=(const sockaddr_in& a, const sockaddr_in& b)
{
	return a.sin_port != b.sin_port && a.sin_addr.s_addr != b.sin_addr.s_addr;
}

extern "C"
{
	MYSHARED_API Server* CreateServer();
	MYSHARED_API void    SRun(Server* p_obj);
	MYSHARED_API void	 SClose(Server* p_obj);
	MYSHARED_API int	 SSend(Server* p_obj,sockaddr_in p_client);
	MYSHARED_API int	 SSendBis(Server* p_obj, char* p_buffer, int size);
	MYSHARED_API int	 SListen(Server* p_obj);
	MYSHARED_API bool	 SStart(Server* p_obj,unsigned int p_port);
	MYSHARED_API ClientInfo SClientInfo(Server* p_obj, int index);
}
