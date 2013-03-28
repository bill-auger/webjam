
#include <fstream>

#include "guiserver.h"
#include "../njclient.h"


extern NJClient *g_client ;


IPageGenerator* GuiServer::onConnection(JNL_HTTPServ *serv , int port)
{
	serv->set_reply_header(HTTP_REPLY_SERVER) ;
	if (!strcmp(serv->get_request_file() , "/load.js")) // via remote script (poll)
	{
//printf("%dload.js" , N++) ;

		// the remote load script that contains the ninjam:// link that triggered this app
		//		polls us until we reply that the js client can be requested confidently
		serv->set_reply_header(HTTP_REPLY_JS) ;
		serv->set_reply_string(HTTP_REPLY_200) ; serv->send_reply() ;
		return new MemPageGenerator(strdup("loadGui() ;")) ;
	}
	else if (!strcmp(serv->get_request_file() , "/")) // via remote script (load gui.js)
	{
		serv->set_reply_header(HTTP_REPLY_HTML) ;
		serv->set_reply_string(HTTP_REPLY_200) ; serv->send_reply() ;

		// read CLIENT_HTML file and return failure message on i/o error
		std::ifstream clientHtmlIfs(CLIENT_HTML) ; std::stringstream clientHtmlSs ;
		if (!clientHtmlIfs.good()) return new MemPageGenerator(strdup("file not found")) ;

		// else return initial client gui page
		clientHtmlSs << clientHtmlIfs.rdbuf() ;
		return new MemPageGenerator(strdup(clientHtmlSs.str().c_str())) ;
	}
	else if (!strcmp(serv->get_request_file() , "/events")) // via local gui.js events
	{
		serv->set_reply_header(HTTP_REPLY_TEXT) ;
		serv->set_reply_string(HTTP_REPLY_200) ; serv->send_reply() ;

		std::stringstream outputJSON ; outputJSON << "{" ;

		// process gui input events
		onMetroMuteEvent(serv , &outputJSON) ;

		// return gui output events
		outputJSON << "}" ;
		return new MemPageGenerator(strdup(outputJSON.str().c_str())) ;
	}
	else // invalid req
	{
//char* t = serv->get_request_file() ; printf("NOT FOUND") ;

		serv->set_reply_string(HTTP_REPLY_404) ; serv->send_reply() ; //return 0 ; // no data
return new MemPageGenerator(strdup("404 NOT FOUND")) ;
	}
}

// incoming gui events
void GuiServer::onMetroMuteEvent(JNL_HTTPServ *serv , std::stringstream* outputJSON)
{
	char* eventData = serv->get_request_parm(METROMUTE_SIGNAL) ; if (!eventData) return ;

//TODO: as we are async - let's be specific and use eventData here
	g_client->config_metronome_mute = !g_client->config_metronome_mute ;
	*outputJSON << METROMUTE_SIGNAL << ":" << (!g_client->config_metronome_mute)? "1" : "0" ;
}
