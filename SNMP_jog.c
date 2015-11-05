/* SNMP_jog.c
 * Kenneth Cross
 * Mason Itkin
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <stdio.h>

// Globals for Session and Response
netsnmp_session session, *ss; // holds connection info.
netsnmp_pdu *response; // holds info. that the remote host sends back

// Functions
int getPdu(char* desc);
int getNextPdu(char* desc);
int getNextPdu_oid(oid* lastOID, size_t lastOID_len); // call getNext with formatted OID
void printInter();

void init_session(netsnmp_session ses, netsnmp_session *sesp, netsnmp_pdu *rep){
    session = ses;
    ss = sesp;
    response = rep;
} //init_session()

int getPdu(char* desc){
    netsnmp_pdu *pdu; // Protocol Data unit - holds info to send to remote host
    oid anOID[MAX_OID_LEN]; // OID for the remote host Size: 'MAX_OID_LEN'
    size_t anOID_len = MAX_OID_LEN;

    netsnmp_variable_list *vars; // List of var to manipulate via SNMP
    int status;
    int count=1;
    
    /*
     * Create the PDU for the data for our request. (Creating SNMP-Get PDU)
     *   1) We're going to GET the system.sysDescr.0 node.
     */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    get_node(desc, anOID, &anOID_len); 
    snmp_add_null_var(pdu, anOID, anOID_len); // Appends NULL to oid
  
    // Send the Request out!!!
    status = snmp_synch_response(ss, pdu, &response);
    
    // Process the response.
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) { 
       // SUCCESS: Print the result variables (Only 1 for now)
    } else {
       // FAILURE: print what went wrong!
      if (status == STAT_SUCCESS){
        fprintf(stderr, "DEBUG:: Error in packet\nReason: %s\n", 
                snmp_errstring(response->errstat));
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

    // Clean up Response (TURNED OFF SO WE CAN USE THE RESPONSE LATER)
    //if (response) //(1)
    //  snmp_free_pdu(response);
} // END the CREATE SNMPGET PDU Function

int getNextPdu(char* desc){
    netsnmp_pdu *pdu; // Protocol Data unit - holds info to send to remote host
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    
    /*
     * Create the PDU for the data for our request. (Creating SNMP-Get PDU)
     *   1) We're going to GET the system.sysDescr.0 node.
     */ 
    // pdu create function and Type of SNMP_MSG_GETNEXT
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    
    if (!snmp_parse_oid(desc, anOID, &anOID_len)) { 
      snmp_perror(desc);
      exit(1); 
    }

    snmp_add_null_var(pdu, anOID, anOID_len);
  
    // Send the Request out!!!
    status = snmp_synch_response(ss, pdu, &response);
    
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) { 
      // Success
      return 1;

    } else {
       // FAILURE: print what went wrong!

      if (status == STAT_SUCCESS){
        fprintf(stderr, "DEBUG:: Error in packet\nReason: %s\n", 
                snmp_errstring(response->errstat));
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
    netsnmp_pdu *pdu;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;

    netsnmp_variable_list *vars;
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
       // FAILURE: print what went wrong!
      if (status == STAT_SUCCESS){
        fprintf(stderr, "DEBUG:: Error in packet\nReason: %s\n", 
                snmp_errstring(response->errstat));
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
} // getNextPdu_oid()

void printInter(){
  char rD[] = "ifType.1";
  int count = 49; //Start at 1 (ASCII)
  int temp =0;
  int loopbackIf = -1;

  printf("\n*****INTERFACE LIST:*****\n\n");

  while(getPdu(rD) == 1){
    printf("**INTERFACE %d:**\n", count-48);

    print_variable(response->variables->name, response->variables->name_length, 
            response->variables);

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
    print_variable(response->variables->name, response->variables->name_length, 
            response->variables);

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
        print_variable(response->variables->name, response->variables->name_length, 
                response->variables);
        getNextPdu_oid(response->variables->name, response->variables->name_length);
      }
    }
    else
      printf("No IP neighbors for the local loopback adapter...\n");

    getNextPdu_oid(response->variables->name, response->variables->name_length);
  }

  printf("\n*****END INTERFACE LIST:*****\n\n");
} // printInter()
