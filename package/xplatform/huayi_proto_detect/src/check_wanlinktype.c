#include     <stdio.h>  
#include     <stdlib.h>   
#include     <signal.h>
#include     <unistd.h>    
#include     <sys/types.h>  
#include     <sys/stat.h>  
#include     <fcntl.h>   
#include     <termios.h>  
#include     <errno.h>  
#include     <sys/wait.h>
#include     <net/if.h>
#include     "common.h"
#include     "huayi_uci_fun.h"


/**@internal
 * @brief Handles SIGCHLD signals to avoid zombie processes
 *
 * When a child process exits, it causes a SIGCHLD to be sent to the
 * process. This handler catches it and reaps the child process so it
 * can exit. Otherwise we'd get zombie processes.
 */
void _sigchld_handler(int s)
{
	int	status;
	pid_t rc;
	
	printf("Handler for SIGCHLD called. Trying to reap a child\n");

	rc = waitpid(-1, &status, WNOHANG);

	printf("Handler for SIGCHLD reaped child PID %d\n", rc);
}

/** Use this function anytime you need to exit */
void _termination_handler(int s)
{
	/*process exit*/
	printf("NetworkProbe Exiting...");
	exit(s == 0 ? 1 : 0);
}

void alerm_signal(int sig)
{
	printf("sig_alarm \n");
	pcap_breakloop(pcap_handle); 
	printf(" have break loop!\n");
}

/** @internal  * Registers all the signal handlers */
static void init_signals(void) {	
	struct sigaction sa;	
	printf("Initializing signal handlers\n");

	sa.sa_handler = _sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		printf("sigaction(): %s\n", strerror(errno));
		exit(1);
	}

	/* Trap SIGPIPE */
	/* This is done so that when libhttpd does a socket operation on
	 * a disconnected socket (i.e.: Broken Pipes) we catch the signal
	 * and do nothing. The alternative is to exit. SIGPIPE are harmless
	 * if not desirable.
	 */
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) == -1) {
		printf("sigaction(): %s\n", strerror(errno));
		exit(1);
	}

	sa.sa_handler = _termination_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	/* Trap SIGTERM */
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		printf("sigaction(): %s\n", strerror(errno));
		exit(1);
	}

	/* Trap SIGQUIT */
	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		printf("sigaction(): %s", strerror(errno));
		exit(1);
	}
	
	sa.sa_handler = alerm_signal;
	sigemptyset(&sa.sa_mask);	sa.sa_flags = SA_RESTART;
	
	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		printf("SIGALRM sigaction(): %s\n", strerror(errno));
		exit(1);	
	}
}

int __check_wannet_type()
{
	int ret;
	char devname[OPTION_STR_LEN] = "eth1";

	huayi_uci_get_str(PKG_NETWORK,"wan","ifname",devname);

	ret = pppoe_check(devname);
	if(PPPOE_WANNET_TYPE == ret)
		return PPPOE_WANNET_TYPE;
	printf("*****%s*****%d****\n",__FUNCTION__,__LINE__);
	ret = dhcp_check(devname);
	printf("*****%s*****%d****,ret = %d\n",__FUNCTION__,__LINE__, ret);
	if(DHCP_WANNET_TYPE == ret)
		return DHCP_WANNET_TYPE;

	return 0;
}

int check_wannet_type()
{
	int ret;
	char cmd[128] = {0};
	
	ret = __check_wannet_type();
	snprintf(cmd,sizeof(cmd),"huayi_cmd set system wan_proto %d\n",ret);
	system(cmd);
	#if 0
	if(PPPOE_WANNET_TYPE == ret) {
		
	} else if(DHCP_WANNET_TYPE == ret) {
		
	} else {
		
	}
	#endif
	return 0;
}

int main()  
{
	int ret;
	
	init_signals();
	ret = check_wannet_type();
	if(ret == 0)
		exit(0);
}  
