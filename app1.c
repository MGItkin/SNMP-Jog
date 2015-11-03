#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <stdio.h>

/* change the word "define" to "undef" to try the (insecure) SNMPv1 version */
#undef DEMO_USE_SNMP_VERSION_3

#ifdef DEMO_USE_SNMP_VERSION_3
const char *our_v3_passphrase = "The Net-SNMP Demo Password";
#endif

// Globals for Session and Response
netsnmp_session session, *ss; // holds connection info.
netsnmp_pdu *response; // holds info. that the remote host sends back

void getNext_pdu(char* desc);

int main(int argc, char ** argv)
{

    char* mibVar[25];

    printf("CS 158B Assignment#2  (Mason & Kenny):\n\n");    

    /*
     * Initialize the SNMP library
     */
    init_snmp("snmpdemoapp");

    /*
     * Initialize a "session" that defines who we're going to talk to
     */
    printf("Init Session with peer: Localhost...\n\n");
    snmp_sess_init( &session );                   /* set up defaults */
    session.peername = strdup("localhost");
    // 127.0.0.1:161

    /* set up the authentication parameters for talking to the server */

#ifdef DEMO_USE_SNMP_VERSION_3  // NOT USING THIS DEF (SNMP V1 Instead)

    /* Use SNMPv3 to talk to the experimental server */

    /* set the SNMP version number */
    session.version=SNMP_VERSION_3;
        
    /* set the SNMPv3 user name */
    session.securityName = strdup("MD5User");
    session.securityNameLen = strlen(session.securityName);

    /* set the security level to authenticated, but not encrypted */
    session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;

    /* set the authentication method to MD5 */
    session.securityAuthProto = usmHMACMD5AuthProtocol;
    session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
    session.securityAuthKeyLen = USM_AUTH_KU_LEN;

    /* set the authentication key to a MD5 hashed version of our
       passphrase "The Net-SNMP Demo Password" (which must be at least 8
       characters long) */
    if (generate_Ku(session.securityAuthProto,
                    session.securityAuthProtoLen,
                    (u_char *) our_v3_passphrase, strlen(our_v3_passphrase),
                    session.securityAuthKey,
                    &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
        snmp_perror(argv[0]);
        snmp_log(LOG_ERR,
                 "Error generating Ku from authentication pass phrase. \n");
        exit(1);
    }
    
#else /* we'll use the insecure (more simple) SNMPv1 */ 

    /* set the SNMP version number */
    session.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication (NOT SECURE) */
    session.community = "public";
    session.community_len = strlen(session.community);

#endif

    /*
     * Open the session
     */
    SOCK_STARTUP; // Does nothing on UNIX systems (noop - No operation)
    
    ss = snmp_open(&session);                     /* establish the session */
 
    // If session open fails
    if (!ss) { 
      snmp_sess_perror("ack", &session);
      SOCK_CLEANUP;
      exit(1);
    }   

    // call PDU getNext Creation
    //mibVar = "hrSystemUptime.0";
    printf("The Current System Uptime is:\n");
    getNext_pdu("hrSystemUptime.0");

    printf("The Agent's System Description is:\n");
    getNext_pdu("sysDescr.0");

    printf("\n\nDEBUG: TESTING\n");
    getNext_pdu("ifType.1");
    getNext_pdu("ifType.2");
    getNext_pdu("ifType.2");

    //TODO - Function repeat the above command until a repeat is found, then display a count and values.

    // Find a betterway to read and manipulate the output
    

    // Close the Session
    snmp_close(ss); // (2)

    SOCK_CLEANUP; // NOT needed for UNIX (noop) only for windows32
    
    return (0);
} /* main() */


void getNext_pdu(char* desc){

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
    
/*
    // Option-1: Manually give the full qualified OID to 'anOID' and check if fails

    if (!snmp_parse_oid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len)) { 
      snmp_perror(".1.3.6.1.2.1.1.1.0");
      SOCK_CLEANUP;
      exit(1); 
    } */

    // Option-2: Same as above but with no eror checking
    // read_objid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len); // put request OID in 'anOID' var
    
    // Option-3: transform text-identifier to a fully qualified OID to reference that data
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
      /*
       * SUCCESS: Print the result variables (Only 1 for now)
       */

       
      for(vars = response->variables; vars; vars = vars->next_variable)
        print_variable(vars->name, vars->name_length, vars);

      // Might be able to get value from using response->variables->val->integer/string
      // val(net_snmpvardata), val_len, type, name

      //container to hold part of the previous reponse would be helpful to know if a dupe exists


      /* Start Manipulating the information ourselves */
      for(vars = response->variables; vars; vars = vars->next_variable) {

        if (vars->type == ASN_OCTET_STR) {
      char *sp = (char *)malloc(1 + vars->val_len);
      memcpy(sp, vars->val.string, vars->val_len);
      sp[vars->val_len] = '\0';
          printf("value #%d is a string: %s\n", count++, sp);
      free(sp);
    }
        else
          printf("value #%d is NOT a string! Ack!\n", count++);
      }
      /* Finished Manipulating the information */

    } else {
      /*
       * FAILURE: print what went wrong!
       */

      if (status == STAT_SUCCESS)
        fprintf(stderr, "Error in packet\nReason: %s\n",
                snmp_errstring(response->errstat));
      else if (status == STAT_TIMEOUT)
        fprintf(stderr, "Timeout: No response from %s.\n",
                session.peername);
      else
        snmp_sess_perror("snmpdemoapp", ss);

    }

    

    /*
     * Clean up Response
     */
    if (response) //(1)
      snmp_free_pdu(response);

    // END the CREATE SNMPGET PDU Function
}