
#include <fstream>

#include "guiserver.h"
#include "../njclient.h"


extern NJClient *g_client ;
extern WDL_PtrList<char> g_chat_queue ;

IPageGenerator* GuiServer::onConnection(JNL_HTTPServ *serv , int port)
{
	char* event = serv->get_request_file() ; event++ ;
	char* data = serv->get_request_parm(DATA_KEY) ; bool isData = (data && *data) ;
	serv->set_reply_header(HTTP_REPLY_SERVER) ;

	// via remote script (poll)
	if (!strcmp(event , PING_SIGNAL))
	{
//printf("%dload.js" , N++) ;

		remoteUrl = (isData && !strncmp(data , "http" , 4))? std::string(data) : std::string(DEFAULT_URL) ; // remote url - store this for return when app closes

		// the remote load script that contains the ninjam:// link that triggered this app
		//		polls us until we reply that the js client can be requested confidently
		serv->set_reply_header(HTTP_REPLY_JS) ;
		serv->set_reply_string(HTTP_REPLY_200) ; serv->send_reply() ;
		return new MemPageGenerator(strdup(PONG_SIGNAL)) ;
	}

	// via remote script "loadGui()"
	if (!strcmp(event , GET_SIGNAL))
	{
		serv->set_reply_header(HTTP_REPLY_HTML) ;
		serv->set_reply_string(HTTP_REPLY_200) ; serv->send_reply() ;

		// read CLIENT_HTML file and return failure message on i/o error
		std::ifstream clientHtmlIfs(CLIENT_HTML) ; std::stringstream clientHtmlSs ;
		if (!clientHtmlIfs.good()) return new MemPageGenerator(strdup("file not found")) ;

		// else return main client gui html
		clientHtmlSs << clientHtmlIfs.rdbuf() ;
		return new MemPageGenerator(strdup(clientHtmlSs.str().c_str())) ;
	}

	// the remaining cases process local gui input and output events
	serv->set_reply_header(HTTP_REPLY_TEXT) ;
	serv->set_reply_string(HTTP_REPLY_200) ; serv->send_reply() ;
	std::stringstream outputJSON ; outputJSON << "{" ;

	if (!strcmp(event , INIT_SIGNAL)) returnInitState(&outputJSON) ;
	else if (!strcmp(event , METROMUTE_SIGNAL))
		{ handleMetroMuteEvent() ; returnMetroMuteState(&outputJSON) ; }
	else if (!strcmp(event , CHAT_SIGNAL) && isData) handleChatEvent(data) ;

	returnCoreState(&outputJSON) ;
	const std::string& outStr = outputJSON.str() ; unsigned int len = outStr.length() ;
	char out[len + 1] ; sprintf(out , outStr.c_str()) ; out[len - 1] = '}' ;
	return new MemPageGenerator(strdup(out)) ;
}

void GuiServer::returnInitState(std::stringstream* outputJSON)
{
	returnRemoteUrl(outputJSON) ;
	returnBpmState(outputJSON) ;
	returnBpiState(outputJSON) ;
	returnMasterVolState(outputJSON) ;
	returnMasterPanState(outputJSON) ;
	returnMasterMuteState(outputJSON) ;
	returnMetroVolState(outputJSON) ;
	returnMetroPanState(outputJSON) ;
	returnMetroMuteState(outputJSON) ;
}

// remote url
void GuiServer::returnRemoteUrl(std::stringstream* out)
	{ *out << URL_SIGNAL << ":'" << remoteUrl << "'," ; }

/// bpm & bpi
void GuiServer::returnBpmState(std::stringstream* out)
	{ *out << BPM_SIGNAL << ":" << g_client->GetActualBPM() << "," ; }

void GuiServer::returnBpiState(std::stringstream* out)
	{ *out << BPI_SIGNAL << ":" << g_client->GetBPI() << "," ; }

// master
void GuiServer::returnMasterVolState(std::stringstream* out)
	{ *out << MASTERVOL_SIGNAL << ":" << g_client->config_mastervolume << "," ; }

void GuiServer::returnMasterPanState(std::stringstream* out)
	{ *out << MASTERPAN_SIGNAL << ":" << g_client->config_masterpan << "," ; }

void GuiServer::returnMasterMuteState(std::stringstream* out)
	{ *out << MASTERMUTE_SIGNAL << ":" << g_client->config_mastermute << "," ; }

// metronome
void GuiServer::returnMetroVolState(std::stringstream* out)
	{ *out << METROVOL_SIGNAL << ":" << g_client->config_metronome << "," ; }

void GuiServer::returnMetroPanState(std::stringstream* out)
	{ *out << METROPAN_SIGNAL << ":" << g_client->config_metronome_pan << "," ; }

void GuiServer::handleMetroMuteEvent()
	{ g_client->config_metronome_mute = !g_client->config_metronome_mute ; }

void GuiServer::returnMetroMuteState(std::stringstream* out)
	{ *out << METROMUTE_SIGNAL << ":" << ((g_client->config_metronome_mute)? "1" : "0") << "," ; }

// locals
//const char *sname=g_audio->GetChannelName(sch);

// chat
void GuiServer::handleChatEvent(char* data) {
unsigned int len = strlen(data) + 1 ;
		char decoded[len] ; url_decode(data , decoded , len) ;

	g_client->ChatMessage_Send("MSG" , strdup(decoded)) ; }

//void handleChatPvtEvent(char* destFullUserName , char* chatMsg) { g_client->ChatMessage_Send("PRIVMSG" , destFullUserName , chatMsg) ; }

// core
void GuiServer::returnCoreState(std::stringstream* out)
{
	// interval progress
	int bpi = g_client->GetBPI() ; int pos , len ; g_client->GetPosition(&pos , &len) ;
	*out << PROGRESS_SIGNAL << ":" << ((pos * bpi) / len) << "," ;

/*TODO: beat message
	char output[11];
	snprintf(output, sizeof(output), "%d", (pos*bpi)/len);
	Glib::ustring beatmsg = output;
	beatmsg += "/";
	snprintf(output, sizeof(output), "%d", bpi);
	beatmsg += output;
	beatmsg += " @ ";
	snprintf(output, sizeof(output), "%.1f", g_client->GetActualBPM());
	beatmsg += output;
	beatmsg += _(" BPM ");
	progressbar1->set_text(beatmsg);
*//* master VU
	float value = VAL2DB(g_client->GetOutputPeak());
	progressbar_master->set_fraction((value+120)/140);
	snprintf(output, sizeof(output), "%.2f dB", value);
	progressbar_master->set_text(output);
*//* user VUs
	vbox_local->update_VUmeters();
	vbox_remote->update_VUmeters();
	if (g_client->HasUserInfoChanged())
	vbox_remote->update();
*/

	// chat messages
	int chatN = g_chat_queue.GetSize() ;
	while (chatN--)
	{
		// TODO: url_encode() in this class segfaults on certain chars
		//		but the only char  likely to be problematic for js client
		//		is the single-quote (our delimiter) so we will just replace them with "&apos;"
		char* chatItem = g_chat_queue.Get(0) ; std::string chatStr(chatItem) ;
		unsigned found = chatStr.find_first_of("'") ;
		while (found != std::string::npos)
			found = chatStr.replace(found , 1 , "&apos;").find_first_of("'" , found + 1) ;
		*out << CHAT_SIGNAL << ":'" << chatStr << "'," ;
		free(chatItem) ; g_chat_queue.Delete(0) ;
/*
		char* chat = g_chat_queue.Get(0) ;
		*out << CHAT_SIGNAL << ":'" << strdup(chat) << "'," ;
		free(chat) ; g_chat_queue.Delete(0) ;
*/
	}
}
