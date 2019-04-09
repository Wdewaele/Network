#include "Client.h"


Client::Client()
	: m_handle(INVALID_SOCKET), m_serverHint(), m_state(State::DISCONNECTED)
{}

Client::~Client()
{
	if (m_handle != INVALID_SOCKET)
		closesocket(m_handle);
	WSACleanup();
}

bool Client::AttemptToConnect()
{
	m_state = State::CONNECTING;

	int bytesSent = Send(nullptr, 0, PacketType::CONNECTION);

	if(Receive() < 1)
	{
		m_state = State::DISCONNECTED;
		return false;
	}
	PacketType pt = static_cast<PacketType>(*reinterpret_cast<uint8_t*>(&m_buffer));
	if(pt ==  PacketType::RETURN_CONNECTION)
	{
		m_id = static_cast<int>(*(m_buffer + 1));
		m_state = State::CONNECTED;
		return true;
	}
	return false;
}

int Client::Send(char* data, int size, PacketType type)
{
	char* temp = new char[size+1];
	memcpy(temp, &type, 1);
	memcpy(temp + 1, data, size);

	int bytesSent = sendto(m_handle, temp, size + 1, 0, reinterpret_cast<sockaddr*>(&m_serverHint), sizeof(m_serverHint));
	if (bytesSent < 0)
	{
		gDebugCallback("Failed to send properly");
		delete temp;
		return -1;
	}
	delete temp;
	return bytesSent;	
}

int Client::GetID()
{
	return m_id;
}

void Client::Close()
{
	if (m_handle != INVALID_SOCKET)
		closesocket(m_handle);
	WSACleanup();
}

bool Client::Start(unsigned p_port)
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);

	if (wsOk != 0)
	{
		gDebugCallback("Can't start the Winsock Lib.");
		return false;
	}
	
	m_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bool broadcast = true;

	if (setsockopt(m_handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast)) < 0)
		Close();

	if (m_handle == SOCKET_ERROR)
	{
		gDebugCallback("failed to create socket\n");
		return false;
	}

	sockaddr_in clienthint;

	clienthint.sin_family = AF_INET;
	clienthint.sin_addr.s_addr = INADDR_ANY;
	clienthint.sin_port = 0;

	m_serverHint.sin_family = AF_INET;
	m_serverHint.sin_addr.s_addr = INADDR_BROADCAST;
	m_serverHint.sin_port = htons(p_port); // 9843

	if (bind(m_handle, reinterpret_cast<const sockaddr*>(&clienthint), sizeof(sockaddr_in)) < 0)
	{
		gDebugCallback("failed to bind socket.\n");
		return false;
	}

	DWORD nonblocking = 1;
	if (ioctlsocket(m_handle, FIONBIO, &nonblocking) != 0)
	{
		gDebugCallback( "failed to set non blocking.\n");
		return false;
	}
	return true;
}

int Client::Receive()
{
	int serverLength = sizeof(m_serverHint);

	ZeroMemory(&m_serverHint, serverLength);
	ZeroMemory(m_buffer, 254);

	int bytesReceived = recvfrom(m_handle, m_buffer, sizeof(m_buffer), 0, reinterpret_cast<sockaddr*>(&m_serverHint), &serverLength);
	if (bytesReceived < 0)
		return -1;
	return bytesReceived;
}

int Client::Received(char* buffer, int size)
{
	int serverLength = sizeof(m_serverHint);
	int bytesReceived = recvfrom(m_handle, buffer, size, 0, reinterpret_cast<sockaddr*>(&m_serverHint), &serverLength);
	
	if (bytesReceived < 0)
		return -1;
	return bytesReceived;
}

uint8_t Client::GetState()
{
	return static_cast<uint8_t>(m_state);
}

char* Client::GetBuffer()
{
	return m_buffer;
}


MYSHARED_API Client* CreateClient()
{
	return new Client();
}

MYSHARED_API bool	 CStart(Client* p_obj, unsigned int p_port)
{
	return p_obj->Start(p_port);
}

MYSHARED_API bool CAttemptToConnect(Client* p_obj)
{
	if(!p_obj)
	{
		std::cout << "failed to start the function, socket isn't available.\n";
		return false;
	}
	return p_obj->AttemptToConnect();
}

MYSHARED_API int	 CReceive(Client* p_obj)
{
	if (!p_obj)
	{
		std::cout << "failed to start the function, socket isn't available.\n";
		return -1;
	}
	return p_obj->Receive();
}

MYSHARED_API int	 CSend(Client* p_obj, char* p_data, int size)
{
	if (!p_obj)
	{
		std::cout << "failed to start the function, socket isn't available.\n";
		return -1;
	}
	return p_obj->Send(p_data, size);
}

MYSHARED_API uint8_t GetState(Client* p_obj)
{
	return p_obj->GetState();
}

MYSHARED_API void	 CClose(Client* p_obj)
{
	p_obj->Close();
}

MYSHARED_API int CGetID(Client* p_obj)
{
	return	p_obj->GetID();
}

MYSHARED_API char* CGetBuffer(Client* p_obj)
{
	return p_obj->GetBuffer();
}

MYSHARED_API int CReceived(Client* p_obj, char* p_buffer, int size)
{
	return p_obj->Received(p_buffer, size);
}
