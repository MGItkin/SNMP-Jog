/* test_monitor.c
 * Kenneth Cross
 * Mason Itkin
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "SNMP_jog.c"

//#include "monitor.c"
// All the monitor helper functions go in the monitor.c file

// Globals for Session and Response
netsnmp_session session, *ss; // holds connection info.
netsnmp_pdu *response; // holds info. that the remote host sends back

int main(int argc, char *argv[]){
  int seconds, samples;
  char *community, *hostIP;

  if (argc < 4){
    printf("\n##########USING DEFAULTS#############\n\n");
    seconds = 500;
    samples = 30;
    community = "public";
    hostIP = "127.0.0.1";
  } else{
    seconds = (int)argv[0]; //How do i cast?
    samples = (int)argv[1];
    community = argv[2];
    hostIP = argv[3];
  }

  printf("%s %d\n", community, samples);

  char* mibVar[25];

  init_snmp("test_monitor");

  // Initialize a "session" that defines who we're going to talk to
  snmp_sess_init( &session );
  session.peername = strdup("localhost");

  session.version = SNMP_VERSION_1;
  session.community = "public";
  session.community_len = strlen(session.community);

  // initalize variables in SNMP-jog
  init_session(session, ss, response);

   // Open the session
  ss = snmp_open(&session);
  if (!ss) { 
    snmp_sess_perror("Failed to open session", &session);
    exit(1);
  }   

  //print_variable(response->variables->name, response->variables->name_length, response->variables); //an alternative way to print info
  printf("The Agent's System Description is:\n");
  getPdu("sysDescr.0");

  //printInter();

  /* 
   * The key is to next grab all the objects and store them
   * in order to poll continuously
   */

  // Close the Session
  snmp_close(ss); // (2)

  return(0);
}
