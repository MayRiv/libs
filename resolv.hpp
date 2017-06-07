#pragma once


#include "socket.hpp"
#include <sys/socket.h>
#include <resolv.h>
#include <string>
#include <vector>
#include "storage.hpp"


namespace lb
{



class Resolv
{
public:
	Resolv()
	{
	}

	class Error {};
	class TryAgain {};
	class NoData {};
	class NotFound {};

	static const int RESOLV_RETRANS = 2;
	static const int RESOLV_RETRY = 2;

	class Request
	{
	public:
		Request(const int req)
			: _req(req)
		{
		}
		virtual ~Request()
		{
		}
		virtual void Parse(Storage &st, const Word32 mLen) = 0;
		const int GetReq() const
		{
			return _req;
		}
		void SetTTL(int ttl)
		{
			_ttl = ttl;
		}
	protected:
		int _req;
		int _ttl;
	};

	class MxRequest : public Request
	{
	public:
		MxRequest()
			: Request(T_MX)
		{
		}
		virtual void Parse(Storage &st, const Word32 mLen);
		class MxRecord
		{
		public:
			MxRecord(const std::string &host, const Word16 weight)
				: host(host), weight(weight)
			{
			}
			std::string host;
			Word16 weight;
		};
		typedef std::vector<MxRecord> TMxList;
		TMxList &MXs()
		{
			return _mxs;
		}
	private:
		
		TMxList _mxs;
	};

	class TxtRequest : public Request
	{
	public:
		TxtRequest()
			: Request(T_TXT)
		{
		}
		RString &Txt()
		{
			return _txt;
		}
		virtual void Parse(Storage &st, const Word32 mLen);
	private:
		RString _txt;
	};

	class PtrRequest : public Request
	{
	public:
		PtrRequest()
			: Request(T_PTR)
		{
		}
		String &Ptr()
		{
			return _ptr;
		}
		virtual void Parse(Storage &st, const Word32 mLen);
	private:
		String _ptr;
	};

	void Get(const std::string &domain, Request &req, Storage &st);
private:
	static const Word32 MAX_ANSWER_SIZE = 2000;
	class AutoResState
	{
	public:
		AutoResState()
		{
			_resState.options = RES_DEFAULT;
			_resState.retrans = RESOLV_RETRANS;
			_resState.retry = RESOLV_RETRY;
			res_ninit(&_resState);
		}
		~AutoResState()
		{
			res_nclose(&_resState);
		}
		struct __res_state *Res()
		{
			return &_resState;
		}
	private:
		struct __res_state _resState;
	};
};

};
