void misc_print(int priority, const char* format, ... );
int  misc_set_priority(int priority);
int  misc_check_args_option (int argc, char *argv[], char * option);
int  misc_set_baud(char * serialDevice, int fdSerial, int baud);
int  misc_hostname_to_ip(char * hostname , char* ip);
int  misc_is_ip_addr(char *ipAddr);
void misc_all_notes_off(int fdSerial, int socket_out);
void misc_send_midi_raw(char * buf, int bufLen);
#define FALSE 0
#define TRUE  1
#define STR_TRUE  "True"
#define STR_FALSE "False"
#define TASK_PRIORITY -20
//#define INCLUDE_DEBUG
//#define POLL_SEQ_HAND 
//#define INCLUDE_SERIAL


