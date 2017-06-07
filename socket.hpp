#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "string.hpp"

namespace lb
{
	const int MAX_IP_SIZE					= 16;
	const int SOCK_OP_TIMEOUT			= 20; // socket operation time out in seconds
	const int SOCK_WAIT_REQUEST		= 4;  // time that readHttp wait request data in seconds
	const int MAX_LISTEN_BACKLOG	= 512;

	typedef int Descriptor;
	typedef int BOOL;

	class SocketClient
	{
	public:
		SocketClient *GetConnection();

		int InitServer(int port, const char *listenIP);
		int Send(const char *buf, const int len, const int timeout = SOCK_OP_TIMEOUT, const int flag = 0);
		int SetNonBlock();
		int SetBlock();
		int BindOnIp(const char *bindIP, int port);

		int SendFile(char *filename, char *buf, int bufLength);

		static const int RETURN_RECV_SIZE = 1;
		int ReadBin(char * buf, int len, int timeout=SOCK_OP_TIMEOUT, int flag=0); // Read ful (len) data package
		int ReadHttp(char * buf, int len, int timeOut = SOCK_WAIT_REQUEST, int flag=0); // Read http request
		int ReadHttpPost(char *buf, int len, int flag=0); // Read http post request

		int ReadUntilStr(char *buf, int len, const char *endTemple, int timeout=SOCK_WAIT_REQUEST, int flag=0); // Read until find endTemple
		int ReadUntilStr(RString &buf, const char *endTemple, int timeout, int flag=0); // Read until find endTemple
		int ReadImapAnswer(RString &buf, const char **cmdLabel, int timeout, int flag=0); // Read until find endTemple

		int ReadUntilInnerStr(char *buf, int len, char **pFind, char *endTemple, int timeout=SOCK_WAIT_REQUEST, int flag=0);
		int Read(char *buf, int len, int flag=0, int timeOut = SOCK_WAIT_REQUEST); // Read not max len
		int Connect(const char *host, int port);
		int Connect(unsigned int ip, int port);
		int Connect(unsigned int ip, int port, int nsec);
		int HasError();
		int SetNoDelay(int flag);
		int SetDeferAccept(int val);
		void Shutdown();
		void Close();
		void Open();
		void SetDescriptor(Descriptor descr) { this->descr = descr; }

		SocketClient();
		SocketClient(Descriptor descr, struct sockaddr_in &addr);
		~SocketClient();

		int IsOk(void);
		Descriptor GetDescr(void);
		unsigned int GetIP() {  return fromIP; }
		char * GetIPStr();
		void GetIPStr(class RString &buf);
		static void GetIPStr(const Word32 ip, class RString &buf);
		static char *GetIPStr(unsigned int ip);
		static unsigned int IpToLong(const char *host);
		static unsigned int MthrGetIpStr(const char *host);
		static unsigned int GetIPStr(const char *host);
		friend int operator == (const lb::SocketClient &client, const int &socket);
		friend class Control;

		enum ESocketResult
		{
			OK,
			ERROR,
			RETRY,
			EMPTY_POLL,
			CONN_CLOSE,
			OUT_OF_BUFFER,
			TIME_OUT,
		};
		ESocketResult PollRead(RString &buf, int timeout);
		bool ReadHttpQuery(RString &buf, int timeout);
protected:
		Descriptor	descr;
		void SetIp(struct sockaddr_in &addr)
		{
			fromIP  = ntohl(addr.sin_addr.s_addr);
		};
		int			isOk;
		int IsBadIP(struct sockaddr_in	sock_addr);
		unsigned int fromIP;
	};



	inline int SocketClient::IsOk(void)
	{
		return isOk;
	};

	inline Descriptor SocketClient::GetDescr(void)
	{
		return descr;
	}

	inline int operator==(const lb::SocketClient &client, const int &socket)
	{
		if (client.descr == socket)
			return 1;

	return 0;
}
};
