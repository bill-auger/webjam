
#ifndef _GUI_SERVER_H_
#define _GUI_SERVER_H_

// DEBUG
#define DBG_BYPASS_LICENCE 1

// http reply header strings
#define HTTP_REPLY_SERVER "Server:ninjam_client_gui_server/0.0"
#define HTTP_REPLY_200 "HTTP/1.1 200 OK"
#define HTTP_REPLY_404 "HTTP/1.1 404 NOT FOUND"
#define HTTP_REPLY_JS "Content-Type:text/javascript"
#define HTTP_REPLY_HTML "Content-Type:text/html"
#define HTTP_REPLY_TEXT "Content-Type:text/plain"

// resources
#define CLIENT_HTML "client.html"

// gui messages
#define DATA_KEY "data"
#define PING_SIGNAL "load.js"
#define PONG_SIGNAL "loadGui() ;"
#define GET_SIGNAL ""
#define INIT_SIGNAL "init"
#define BPM_SIGNAL "bpm"
#define BPI_SIGNAL "bpi"
#define METROMUTE_SIGNAL "metromute"
#define CHAT_SIGNAL "chat"
// incoming gui messages (unsure if there would be any unpaired incoming msgs)
//e.g.#define UPVOTE_SIGNAL "upvote"
// outgoing gui messages
#define PROGRESS_SIGNAL "progress"

#ifdef _WIN32
#include <windows.h>
#else
#define Sleep(x) usleep((x)*1000)
#endif
//#include <stdio.h>
#include <sstream>

#include "../../WDL/jnetlib/jnetlib.h"
#include "../../WDL/jnetlib/webserver.h"


class MemPageGenerator : public IPageGenerator
{
  public:
		MemPageGenerator(char* buf , int buf_len = -1)
		{
			m_buf = buf ; m_buf_pos = 0 ;
			if (buf_len >= 0) m_buf_size = buf_len ; else m_buf_size = strlen(buf) ;
		}
    virtual ~MemPageGenerator() { free(m_buf); }

    virtual int GetData(char* buf , int size) // return 0 when done
		{
			int a = m_buf_size - m_buf_pos ;
			if (a < size) size = a ;
			memcpy(buf , m_buf + m_buf_pos , size) ;
			m_buf_pos += size ;
			return size ;
		}

  private:
    char* m_buf ;
    int m_buf_size ;
    int m_buf_pos ;
} ;


class GuiServer : public WebServerBaseClass
{
	public:
		GuiServer() { }

		virtual IPageGenerator *onConnection(JNL_HTTPServ* serv , int port) ;

	private:
		void returnInitState(std::stringstream* outputJSON) ;
		void returnBpmState(std::stringstream* outputJSON) ;
		void returnBpiState(std::stringstream* outputJSON) ;
		void handleMetroMuteEvent(char* data) ;
		void returnMetroMuteState(std::stringstream* outputJSON) ;
		void handleChatEvent(char* data) ;
		void returnCoreState(std::stringstream* outputJSON) ;

//int N = 0 ;
} ;

#endif // _GUI_SERVER_H_
