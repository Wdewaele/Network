#include "Server.h"


Server::Server()
	: m_clients(), m_bufferSize(0)
{
	m_isConnected.fill(false);
}


Server::~Server()
{
	if (m_handle != SOCKET_ERROR)
		closesocket(m_handle);
	WSACleanup();
}

void Server::Run()
{
	while(true)
	{
		int bytesReceived = Listen();
		if(bytesReceived == SOCKET_ERROR)
		{
			gDebugCallback( "Error receiving from client." );
			continue;
		}
	}
}

int Server::Listen()
{
	sockaddr_in client;
	int bytesReceived = Receive(client);
	if (bytesReceived < 0)
		return -1;

	PacketType pt = static_cast<PacketType>(*reinterpret_cast<uint8_t*>(&m_buffer));
	//memcpy(&pt, p_data, 1); same shit but cleaner;

	switch(pt)
	{
	case PacketType::CONNECTION:
		CheckFirstConnection(client);
		gDebugCallback("Answering to the client, Welcome my friend");
		break;

	case PacketType::DATA:
		for(auto iClient : m_clients)
		{
			if (m_receivedAddress != iClient.m_clientAddress)
				Send(iClient.m_clientAddress);
		}
		break;
	
	default: ;
	}
	return bytesReceived;
}

void Server::Close()
{
	if (m_handle != SOCKET_ERROR)
		closesocket(m_handle);
}

int Server::Send(sockaddr_in& p_client)
{
	char* temp = new char[m_bufferSize -1];
	memcpy(temp, m_buffer +1, m_bufferSize -1);
	--m_bufferSize;

	int bytesSent = sendto(m_handle, temp, m_bufferSize, 0, reinterpret_cast<sockaddr*>(&p_client), sizeof(p_client));
	gDebugCallback(std::to_string(bytesSent).c_str());
	
	if (bytesSent < 0)
	{
		delete temp;
		return -1;
	}
	delete temp;
	return bytesSent;
}

int Server::Send(char* p_buffer, int size)
{
	char* temp = new char[size - 1];
	memcpy(temp, p_buffer + 1, size - 1);
	--size;
	int bytesSent = 0;
	for (auto iClient : m_clients)
	{
		if (iClient.m_id != 0 )
			bytesSent = sendto(m_handle, temp, size, 0, reinterpret_cast<sockaddr*>(&iClient.m_clientAddress), sizeof(iClient.m_clientAddress));
	}
	gDebugCallback(std::to_string(bytesSent).c_str());

	if (bytesSent < 0)
	{
		delete temp;
		return -1;
	}
	delete temp;
	return bytesSent;
}


int Server::Receive(sockaddr_in& p_client)
{
	int clientLenght = sizeof(p_client);

	ZeroMemory(&p_client, clientLenght);
	ZeroMemory(m_buffer, 254);

	int bytesReceived = recvfrom(m_handle, m_buffer, sizeof(m_buffer), 0, reinterpret_cast<sockaddr*>(&p_client), &clientLenght);
	if (bytesReceived < 0)
		return -1;
	
	m_receivedAddress = p_client;
	m_bufferSize = bytesReceived;
	return bytesReceived;
}

bool Server::Start(unsigned p_port)
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if(wsOk != 0)
	{
		gDebugCallback( "Failed the initialization of Winsock");
		return false;
	}

	m_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_handle == INVALID_SOCKET)
	{
		gDebugCallback("Invalid socket");
		return false;
	}

	sockaddr_in serverHint;
	serverHint.sin_addr.s_addr = INADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(p_port); //9843

	if(bind(m_handle, reinterpret_cast<sockaddr*>(&serverHint), sizeof(serverHint)) == SOCKET_ERROR)
	{
		gDebugCallback( "Can't bind your Socket.");
		return false;
	}

	ClientInfo server{ serverHint, 0 };
	m_clients[0] = server;

	DWORD nonblocking = 1;
	if (ioctlsocket(m_handle, FIONBIO, &nonblocking) != 0)
	{
		gDebugCallback("failed to set non blocking.\n");
		return false;
	}
	return true;
}

ClientInfo Server::GetClientInfo(int index)
{
	return m_clients[index];
}

void Server::CheckFirstConnection(sockaddr_in& p_client)
{
	int slotClient = FindClient(p_client);
	if(slotClient > -1)
		return;
	
	int ID = FindOpenSlot();
	if(ID == -1 )
	{
		gDebugCallback("already reached the limit of connection." );
		return;
	}
	ClientInfo client{p_client, ID};
	m_clients[ID] = client;

	char buffertoReturn[5];
	PacketType pt = PacketType::RETURN_CONNECTION;

	memcpy(buffertoReturn, &pt, 1);
	*reinterpret_cast<int*>(buffertoReturn + 1) = ID;

	sendto(m_handle, buffertoReturn, 5, 0, reinterpret_cast<sockaddr*>(&p_client), sizeof(p_client));
}

int Server::FindClient(sockaddr_in& p_client)
{
	for (auto i = 1; i < m_clients.size(); ++i)
	{
		if (m_isConnected[i] && m_clients[i].m_clientAddress == p_client)
			return i;
	}
	return -1;
}

int Server::FindOpenSlot()
{
	for (auto i = 1; i < m_isConnected.size(); ++i)
	{
		if (!m_isConnected[i])
			return i;
	}
	return -1;
}

MYSHARED_API Server* CreateServer()
{
	return new Server();
}

MYSHARED_API void    SRun(Server* p_obj)
{
	if(!p_obj)
	{
		gDebugCallback("failed to run.\n");
	}
	p_obj->Run();
}

MYSHARED_API int	 SSend(Server* p_obj, sockaddr_in p_client)
{
	if (!p_obj)
	{
		gDebugCallback("failed to Send.\n");
		return -1;
	}
	return p_obj->Send(p_client);
}

MYSHARED_API int	 SListen(Server* p_obj)
{
	if (!p_obj)
	{
		std::cout << "failed to Receive.\n";
		return 0;
	}
	return p_obj->Listen();
	
}

MYSHARED_API bool	 SStart(Server* p_obj, unsigned int p_port)
{
	if (!p_obj)
	{
		std::cout << "failed to start.\n";
		return false;
	}
	return p_obj->Start(p_port);
}
MYSHARED_API void	 SClose(Server* p_obj)
{
	p_obj->Close();
}

MYSHARED_API int SSendBis(Server* p_obj, char* p_buffer, int size)
{
	return p_obj->Send(p_buffer, size);
}

MYSHARED_API ClientInfo SClientInfo(Server* p_obj, int index)
{
	return p_obj->GetClientInfo(index);
}

//copy /y $(OutDir)Network_UDP.dll $(SolutionDir)..\Assets\Plugins