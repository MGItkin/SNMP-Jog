#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <stdio.h>

// Globals for Session and Response
netsnmp_session session, *ss; // holds connection info.
netsnmp_pdu *response; // holds info. that the remote host sends back

int getPdu(char* desc);

int getNextPdu(char* desc);

int getNextPdu_oid(oid* lastOID, size_t lastOID_len); // call getNext with an already formatted OID
void printInter();

int main(int argc, char ** argv) {
    char* mibVar[25];

    printf("\n\n------ CS 158B Assignment#2  (Mason & Kenny) ------\n\n");    

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
    /* set the SNMP version number */
    session.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication (NOT SECURE) */
    session.community = "public";
    session.community_len = strlen(session.community);

    /*
     * Open the session
     */
    ss = snmp_open(&session);                     /* establish the session */
 
    // If session open fails
    if (!ss) { 
      snmp_sess_perror("ack", &session);
      exit(1);
    }   

    // call a SNMP get function with getPDU
    //mibVar = "hrSystemUptime.0";
    printf("The Current System Uptime is:\n");
    getPdu("hrSystemUptime.0");
    print_variable(response->variables->name, response->variables->name_length, response->variables);


    printf("\nThe Agent's System Description is:\n");
    getPdu("sysDescr.0");
    print_variable(response->variables->name, response->variables->name_length, response->variables);

    printInter(); // Print information about the Agent's interfaaces


    // Close the Session
    snmp_close(ss); // (2)

    return (0);
}


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
    
/*
    // Option-1: Manually give the full qualified OID to 'anOID' and check if fails

    if (!snmp_parse_oid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len)) { 
      snmp_perror(".1.3.6.1.2.1.1.1.0");
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

       
      /*for(vars = response->variables; vars; vars = vars->next_variable)
        print_variable(vars->name, vars->name_length, vars);*/

      //DEBUG
       /*
      printf("DEBUG: NAME print: %d\n", response->variables->name);

      print_variable(response->variables->name, response->variables->name_length, response->variables);

      printf("DEBUG: String print: %s\n", response->variables->val.string);
      printf("DEBUG: Decimal print: %d\n", *response->variables->val.integer);
      */

      // Might be able to get value from using response->variables->val->integer/string
      // val(net_snmpvardata), val_len, type, name

      //container to hold part of the previous reponse would be helpful to know if a dupe exists


      /* Start Manipulating the information ourselves 
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


       Finished Manipulating the information */

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

    /*
     * Clean up Response (TURNED OFF SO WE CAN USE THE RESPONSE LATER)
     */
    //if (response) //(1)
    //  snmp_free_pdu(response);

    // END the CREATE SNMPGET PDU Function
}

int getNextPdu(char* desc){

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
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT); // Using pdu create function and Type of SNMP_MSG_GETNEXT
    
    
    // Option-1: Manually give the full qualified OID to 'anOID' and check if fails

    if (!snmp_parse_oid(desc, anOID, &anOID_len)) { 
      snmp_perror(desc);
      exit(1); 
    }

    // Option-2: Same as above but with no eror checking
    // read_objid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len); // put request OID in 'anOID' var
    
    // Option-3: transform text-identifier to a fully qualified OID to reference that data
    //get_node(desc, anOID, &anOID_len); 
    

    snmp_add_null_var(pdu, anOID, anOID_len); // Adds the now properly formatted OID to the PDU with a NULL value
  
    /*
     * Send the Request out!!!
     */
    status = snmp_synch_response(ss, pdu, &response);
    
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) { 
      // Success
      return 1;

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
        return -3;
    }
    
}

int getNextPdu_oid(oid* lastOID, size_t lastOID_len){
    netsnmp_pdu *pdu; // Protocol Data unit - holds info to send to remote host
    oid anOID[MAX_OID_LEN]; // OID for the info we want from the remote host Size: 'MAX_OID_LEN'
    size_t anOID_len = MAX_OID_LEN; // Assigned in Wiki tut to be: size_t anOID_len = MAX_OID_LEN;  

    netsnmp_variable_list *vars; // List of var to manipulate via SNMP
    int status;
    int count=1;

    // GetNext on a already properly formatted OID
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    snmp_add_null_var(pdu, lastOID, lastOID_len);
    status = snmp_synch_response(ss, pdu, &response);


    // Error checking will only  occur for the second call
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) { 
      // Success
      return 1;

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
        return -3;
    }

}

void printInter(){

  char rD[] = "ifType.1";
  int count = 49; //Start at 1 (ASCII)
  int temp =0;
  int loopbackIf = -1;

  printf("\n*****INTERFACE LIST:*****\n\n");

  while(getPdu(rD) == 1){
    printf("**INTERFACE %d:**\n", count-48);

    print_variable(response->variables->name, response->variables->name_length, response->variables);

    printf("\n");
    snmp_free_pdu(response);
    count++;
    rD[7]=count;
  }

  // back to int values
  count = count - 48;
  temp = 1;

  getNextPdu("ipAdEntAddr");

  for(temp; temp < count; temp++){
    printf("\n**IP of Interface **%d: \n", temp);
    print_variable(response->variables->name, response->variables->name_length, response->variables);

    if(*response->variables->val.bitstring == 127){
      loopbackIf = temp;
    }

    getNextPdu_oid(response->variables->name, response->variables->name_length);

  }

  temp = 1;

  getNextPdu("ipNetToMediaNetAddress");

  for(temp; temp < count; temp++){

    printf("\n**Neighborig IP for Interface %d:**\n", temp);

    if(loopbackIf != temp && *response->variables->val.bitstring == 192){
      while(*response->variables->val.bitstring == 192){
        print_variable(response->variables->name, response->variables->name_length, response->variables);
        getNextPdu_oid(response->variables->name, response->variables->name_length);
      }
    }
      
    else
      printf("No IP neighbors for the local loopback adapter...\n");

    getNextPdu_oid(response->variables->name, response->variables->name_length);

  }

printf("\n*****END INTERFACE LIST:*****\n\n");

printf("\n");


}