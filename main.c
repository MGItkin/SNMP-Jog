/* main.c
 * Kenneth Cross
 * Mason Itkin
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <stdio.h>
#include "SNMP_jog.c"

// Globals for Session and Response
netsnmp_session session, *ss; // holds connection info.
netsnmp_pdu *response; // holds info. that the remote host sends back

int main(int argc, char ** argv) {
    int seconds, samples;
    char *community, *hostIP;
    char* mibVar[25];
  
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


    printf("\n\n------ CS 158B Assignment#2  (Mason & Kenny) ------\n\n");    
    printf("Init Session with peer: Localhost...\n\n");

    init_snmp("SNMP_Jog");
    snmp_sess_init( &session ); /* set up defaults */
    session.peername = strdup("localhost");

    /* set up the authentication parameters for talking to the server */
    session.version = SNMP_VERSION_1;
    session.community = "public";
    session.community_len = strlen(session.community);

    // Open the session
    ss = snmp_open(&session);
    if (!ss) { 
      snmp_sess_perror("ack", &session);
      exit(1);
    }   

    // initalize variables in SNMP-jog
    init_session(session, ss, response);

    // call a SNMP get function with getPDU
    printf("The Current System Uptime is:\n");
    getPdu("hrSystemUptime.0");
    print_variable(response->variables->name, 
            response->variables->name_length, response->variables);

    printf("\nThe Agent's System Description is:\n");
    getPdu("sysDescr.0");
    print_variable(response->variables->name, 
            response->variables->name_length, response->variables);

    printInter(); // Print information about the Agent's interfaaces

    // Close the Session
    snmp_close(ss); // (2)

    return (0);
}
