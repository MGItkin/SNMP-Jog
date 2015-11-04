#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <stdio.h>

// Globals for Session and Response
netsnmp_session session, *ss; // holds connection info.
netsnmp_pdu *response; // holds info. that the remote host sends back

int getPdu(char* desc);
void printInter();

int main(int argc, char ** argv) {
    char* mibVar[25];

    printf("CS 158B Assignment#2  (Mason & Kenny):\n\n");    
    init_snmp("snmpdemoapp");

    /*
     * Initialize a "session" that defines who we're going to talk to
     */
    printf("Init Session with peer: Localhost...\n\n");
    snmp_sess_init( &session );                   /* set up defaults */
    session.peername = strdup("localhost");

    /* set the SNMP version number */
    session.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication (NOT SECURE) */
    session.community = "public";
    session.community_len = strlen(session.community);

    /*
     * Open the session
     */
    ss = snmp_open(&session);
 
    // If session open fails
    if (!ss) { 
      snmp_sess_perror("ack", &session);
      exit(1);
    }   

    // call PDU getNext Creation
    //mibVar = "hrSystemUptime.0";
    printf("The Current System Uptime is:\n");
    getPdu("hrSystemUptime.0");
    print_variable(response->variables->name, response->variables->name_length, response->variables);


    printf("The Agent's System Description is:\n");
    getPdu("sysDescr.0");
    print_variable(response->variables->name, response->variables->name_length, response->variables);

    printInter();

    //TODO - Function repeat the above command until a repeat is found, then display a count and values.

    // Find a betterway to read and manipulate the output
    

    // Close the Session
    snmp_close(ss); // (2)

    return (0);
} /* main() */


int getPdu(char* desc){

    netsnmp_pdu *pdu; // Protocol Data unit - holds info to send to remote host
    oid anOID[MAX_OID_LEN]; // OID for the info we want from the remote host Size: 'MAX_OID_LEN'
    size_t anOID_len = MAX_OID_LEN; // Assigned in Wiki tut to be: size_t anOID_len = MAX_OID_LEN;  

    netsnmp_variable_list *vars; // List of var to manipulate via SNMP
    int status;
    int count=1;
    
    /*
     * Create the PDU for the data for our request. (Creating SNMP-Get PDU)
     *   1) We're going to GET the system.sysDescr.0 node.
     */
    pdu = snmp_pdu_create(SNMP_MSG_GET); // Using pdu create function and Type of SNMP_MSG_GET
    get_node(desc, anOID, &anOID_len); 
    snmp_add_null_var(pdu, anOID, anOID_len); // Adds the now properly formatted OID to the PDU with a NULL value
  
    /*
     * Send the Request out!!!
     */
    status = snmp_synch_response(ss, pdu, &response);

    /*
     * Process the response.
     */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) { 
       // SUCCESS: Print the result variables (Only 1 for now)
    } else {
      /*
       * FAILURE: print what went wrong!
       */

      if (status == STAT_SUCCESS){
        fprintf(stderr, "DEBUG:: Error in packet\nReason: %s\n", snmp_errstring(response->errstat));
        return -1;
      }
      else if (status == STAT_TIMEOUT){

        fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
        return -2;
      }
      else
        snmp_sess_perror("snmpdemoapp", ss);

    }

    return 1;
}    // END the CREATE SNMPGET PDU Function

void printInter(){

  char rD[] = "ifType.1";
  int count = 49; //Start at 1 (ASCII)

  printf("\n*****INTERFACE LIST:*****\n\n");


  while(getPdu(rD) == 1){
    printf("INTERFACE %d:\n", count-48);
    print_variable(response->variables->name, response->variables->name_length, 
            response->variables);
    snmp_free_pdu(response);

    count++;
    rD[7]=count;
  } 

  printf("\n");
}
