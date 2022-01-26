
#include "HardCodedData.h"


extern char* gp_server_address;
extern HANDLE g_client_log;
extern BOOL g_no_errors;
extern int g_server_port;

BOOL write_to_client_log_file(char* message, BOOL is_sent_to_server, BOOL network_message);
void construct_ip_port_str(char* par);
BOOL construct_message_to_log_file(char* par);
BOOL file_opening_client(char* username);
