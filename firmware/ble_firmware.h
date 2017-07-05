#ifndef FIRMWARE_BLE_H
#define FIRMWARE_BLE_H

#include "cube_hal.h"
#include "hal_types.h"
#include "bluenrg_gatt_server.h"
#include "bluenrg_gap.h"
#include "string.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "hci_const.h"
#include "gp_timer.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_aci_const.h"
#include "hci.h"
#include "hci_le.h"
#include "hal.h"
#include "sm.h"
#include "uart_support.h"
#include "stm32_bluenrg_ble.h"
#include "ble_clock.h"
#include "ble_status.h"
#include <list.h>

#ifndef BLE_APP_CONFIG
#include  <blefw_conf.h>
#endif


#ifndef COPY_VAR
#define COPY_VAR(dest,source) memcpy(dest,(void*)source,sizeof((source)))
#endif



#ifndef DEVICE_ADDRS_LENGTH
#define DEVICE_ADDRS_LENGTH     (6)
#endif

//#define MAX_SERVER_ATT_SIZE             0x03


/************************************************APP_BLE DEFINITIONS***************************************/
typedef enum /*<! Used for return the result of an operation commanded by the application firmware >*/
{
  APP_SUCCESS = 0x00,            /*!< command success applied */
  APP_ERROR =  0x01,             /*!< and error occour during the command execution*/
  APP_NOT_CONFIG= 0x02           /*!< not configuration is present before to applied a command*/
} APP_Status; 


typedef enum device_type{/*!< used to specify a valid device role  >*/
  DEVICE_PERISPHERAL,           /*!< device as a perispheral >*/          
  DEVICE_CENTRAL,               /*!< device as a central >*/
  DEVICE_BROADCASTER,           /*!< device as a broadcaster >*/
  DEVICE_OBSERVER               /*!< device as observer >*/ 
}dv_type_t;

/*device status*/
typedef enum device_state{/*!< Used for indicate what is the current status of the device >*/
DEVICE_UNITIALIZED,				     /*!< device_initialized >*/
DEVICE_DISCOVERY_MODE,		     /*!< device set as discovery mode >*/
DEVICE_ADVERTISEMENT_MODE,		 /*!< device set as adverticement mode >*/
DEVICE_SCAN_MODE,		           /*!< device set as scan mode >*/
DEVICE_READY_TO_INTERCHANGE,   /*!< device is ready to  discover services and characterstics  >*/
DEVICE_READY_TO_CONNECT,       /*!< device is ready set up a connection   >*/
DEVICE_READY,				           /*!< device set as ready after connection established >*/
DEVICE_NOT_CONNECTED			     /*!< device set as not connected after many connetion set up tries >*/
}dv_state_t;

typedef struct{/*Used to control the service discovery */
  uint8_t services_to_find;           /*!< How many services have to be scanned >*/  
  uint8_t services_success_scanned;   /*!< Flag fire to indicate that all of the services fot this profile had been scanned >*/  
  uint8_t attr_success_scanned;       /*!< Flag fire once all the characterstics for this profile have been scanned >*/  
  uint8_t notify_success_enable;     /*!< Flag fire once all the notifies for this profile have been enable >*/  
}sv_ctrl_flags;

typedef struct{
uint8_t char_to_scan;							/*!< Amount of characteristics to be scanned >*/
uint8_t char_scanned;							/*!< Amount of characteristics scanned >*/
uint8_t char_discovery_success;   /*!< Flag fire once all the characterstics for this service have been scanned >*/
uint8_t char_notify_enabled_success;   /*!< Flag fire once all the characterstics for this service have been scanned >*/
}char_flags;

struct _app_attr_t{/*This is a general attribute definition*/
  uint8_t CharUUID[16];            /*!< Control characteristic UUID.*/
  uint8_t charUuidType;            /*!< Control characteristic UUID_TYPE. 16 or 128 bits*/   
  uint8_t charValueLen;            /*!< Current length of the characteristic description*/
  uint8_t charProperties;          /*!< Characteristic properties.*/
  uint8_t secPermissions;         /*!< Permissions*/
  uint8_t gattEvtMask;            /*!< Type of event generated by GATT server*/
  uint8_t encryKeySize;           /*!< minimum encription key size requirement for this attr: 7 to 16*/
  uint8_t isVariable;             /*!< 0x00:fixed attr length 0x01:var attr length*/
  uint16_t CharHandle;             /*!< characteristic handle.*/
  uint16_t Associate_CharHandler;   /*!< peer characteristic handle associate to this connection.*/     
  uint8_t n_val;                  /*!< control counter of the number of values associate to this characteristic */
  struct _app_attr_t * next_attr;
};

typedef struct _app_attr_t app_attr_t; /*<! This is the service type definition >*/

struct _app_service_t{/*<!This is a general service definition >*/
  char_flags chrflags;                  /*!< Characteristics control flags for a service. >*/
  uint8_t ServiceUUID[16];              /*!< Control service UUID. >*/
  uint16_t ServiceHandle;               /*!< Service handle. >*/
  uint8_t service_uuid_type;            /*!< Control service UUID_TYPE. 16 or 128 bits >*/
  uint8_t service_type;                 /*!< Type of service (primary or secondary) >*/
  uint8_t max_attr_records;             /*!< Maximum number of att-records that can be add to this service >*/
  uint8_t n_attr;                       /*!< Control counter of the number of attributes add to this service >*/
  struct _app_service_t * next_service;
  app_attr_t * attrs;
};


typedef struct _app_service_t app_service_t; /*<! This is the service type definition >*/


typedef struct{/*This is the application profile definition */
  uint8_t n_service;                  /*!< Control counter of the number of services associate to this application >*/
  sv_ctrl_flags svflags;              /*!< Control flags for the services associated to a profile >*/  
  app_service_t * services;           /*!< Services associated to this profile  >*/     
}app_profile_t;


typedef struct{/*structure for general && selective discovery configuration*/
  uint8_t scan_type;                 /*!<  set the device in active 0x01 or pasive 0x00 scanning>*/ 
  uint16_t sinterval;               /*!<  Time interval between two LE scans:0x0004-0x4000(period) >*/
  uint16_t swindows;                /*!<  Amoung of time for the duration of the LE scan:0x0004-0x4000(length) >*/
  uint8_t ownaddrtype;              /*!<  0x00:public address, 0x01: random device address >*/
  uint8_t fduplicates;              /*!<  0x00 do not  filter duplicates 0x01: filter duplicates >*/ 
  uint8_t Num_whiteList_Entries;    /*!<  number of devices that have to be addded to the whitelist >*/ 
  uint8_t *addr_array;              /*!<  address array will contain the Num_White_list_entries address_type and addresses >*/ 
}app_discovery_t;



typedef struct{/**structure for advertisement configuration*/
uint8_t adveventtype;           /*!<  0x00: connectable undirected adverticement 
                                      0x02: scanable undirect adverticement 
                                      0x03 non connectable undirect adverticement >*/
uint16_t advintervalmin;        /*!<  minimum advertisement interval for non direct advertisement: 0x0020-0x4000 
                                      default N=0x0800: Time=N*0.625ms >*/
uint16_t advintervalmax;        /*!<  minimum advertisement interval for non direct advertisement: 0x0020-0x4000 
                                      default N=0x0800: Time=N*0.625ms >*/
uint8_t advaddrstype;           /*!<  0x00:public address, 0x01: random device address >*/
uint8_t advfilterpoli;          /*!<  0x00:(default)allows scan request and connect request for any device,
                                      0x01:allows scan request for whitelist only but allows connect req for any device 
                                      0x02: scan request for any connect request for white list only 
                                      0x03: scan request and connect request for whitelist only >*/
//uint8_t localnamelength;/*firmware parameter*/
//uint8_t localname[];/*firmware parameter*/
//uint8_t serviceuuidlength;/*user parameter*/
//uint8_t serviceuuidlist[];/*user parameter*/
uint16_t slconnintervalmin;   /*!<  slave connection interval min value 
                                    connection interval min = slconnintervalmin * 1.25ms: 0x006-0x0C80; 
                                    0xFFFF:not specific min>*/
uint16_t slconnintervalmax;   /*!<  slave connection interval min value 
                                    connection interval max = slconnintervalmax * 1.25ms: 0x006-0x0C80;
                                    shall be slconnintervalmax >= slconnintervalmin 
                                    0xFFFF:not specific max >*/
}app_advertise_t;

/********************************************SERVICES HANDLER DEFINITIONS*****************************/

typedef enum{
	SERV_SUCCESS=0x00,
	SERV_ERROR=0x01,
        SERV_NOT_APPL=0x02
}SERV_Status;


typedef enum service_State{
  ST_SERVICE_DISCOVERY,							/*!< Device looking for services >*/
  ST_CHAR_DISCOVERY					      /*!< Device looking for characteristics >*/
}sv_state_t;


typedef struct{/*to delete*/
volatile uint8_t service_discovery;               /*!< this flag is fire when a new service has been discovered >*/    
volatile uint8_t char_discovery;                  /*!< this flag is fire when a new char has been discovered >*/
}sv_hdler_flags;

typedef struct{/*used to configure the device as a server or a client*/
  uint8_t serv_disc_mode;						/*!< this flag is setup for enable/disable the services scanning >*/    
  uint8_t char_disc_mode;						/*!< this flag is setup for enable/disable the characteristic discovery >*/
}servhandler_conf;


typedef struct{/*to delete*/
sv_hdler_flags flags;							/*!< service handler event flags*/
servhandler_conf config;						/*service handler module configuration*/
}service_hdl_t;




/******************************************CONNECTION HANDLER DEFINITIONS**************************************/


typedef enum /*used for return the result of and operation by the connection handler*/
{
  CHADLE_SUCCESS = 0x00,            /*!< command success applied >*/
  CHADLE_ERROR = 0x01,              /*!< and error occour during the command execution >*/
} CHADLE_Status; 

/*connection_status*/
typedef enum connection_State {/*This represent the status of the connection.*/
ST_UNESTABLISHED,					/*!< connection unestablished >*/
ST_STABLISHED,						/*!< connection stablished after discover services and characteristics >*/
ST_OBSERVER,						  /*!<   handler the communication as observer node >*/
ST_BROADCAST, 						/*!< handler the communication as broadcast node >*/
ST_CONNECTED_WAIT_DISC,		/*!< connection wait for interchange services and characterstics >*/
ST_CREATE_CONNECTION,     /*!< connection wait for connection set up >*/
ST_TIME_OUT,            /*!< the connection exceed the time for stablishement >*/
ST_CONNECTION_LOST        /*!< connection lost or impossible to set up >*/
}cn_state_t;

typedef struct{/*structure for connection configuration*/
uint16_t sinterval;               /*!<  Time interval between two LE scans:0x0004-0x4000(period)*/
uint16_t swindows;                /*!<  Amoung of time for the duration of the LE scan:0x0004-0x4000(length)*/
//uint8_t peer_addrtype; /*user parameter*/
//uint8_t peer_addrs [6];/*user parameter*/
uint8_t ownaddrtype;             /*!<  0x00:public address, 0x01: random device address >*/ 
uint16_t cintervalmin;           /*!<  minimum value for connection event interval shall be less or equal to  cintervalmax: 0x0006-0x0C80 >*/
uint16_t cintervalmax;           /*!<  maximum value for connection event interval shall be greater or equal to  cintervalmin: 0x0006-0x0C80>*/
uint16_t clatency;               /*!<  salve latency for connection in number of connection events:0x0000-0x01F4>*/
uint16_t stimeout;               /*!<  supervisor time out for LE link: 0x000A-0x0C80>*/
uint16_t clengthmin;             /*!<  minimum length of connection event needed for LE: 0x0000-0xFFFF time N*0.625ms>*/
uint16_t clengthmax;             /*!<  maximum length of connection event needed for LE: 0x0000-0xFFFF time N*0.625ms>*/
}config_connection_t;


/***************************************NETWORK MODULE DEFINITIONS******************************************/

typedef enum{/*<! Used for return the result of an operation by the connection handler module >*/
	NET_SUCCESS=0x00,			/*!< command success applied >*/
	NET_ERROR=0x01 				/*!< and error occour during the command execution >*/
}NET_Status;

typedef enum{/*define if the network will be operated as a connected or broadcast */
	NET_BROADCAST,			/*!< Network module configured as a broadcast >*/
	NET_CONNECTED				/*!< Network module configured as a master-slave >*/
}net_type_t;




typedef struct{ /*single connection structure*/
uint16_t Connection_Handle;           /*!< define one and only one connection handler x slave  */
app_profile_t * Node_profile;         /*!< could be one profile x slave (most convenient)*/
uint8_t device_type_addrs;            /*!< slave device addrs type*/
uint8_t device_address[6];            /*!< device address val*/
config_connection_t * cconfig;        /*!< device has a special connection configuration(optional) >*/
servhandler_conf  sconfig;            /*!< device has a special services configuration(optional) >*/
cn_state_t connection_status;         /*!< this is the connection status.*/
sv_state_t service_status;

}connection_t;


/*network module control flags*/
typedef struct{/*<! Used for return the result of an operation by the network  module >*/
volatile uint8_t wait_end_procedure;         /*!< This flag is fire when a procedure is not ended >*/
uint8_t connection_stablishment_complete;    /*!< This flag is fire when a procedure is not ended >*/
uint8_t services_discovery_complete;
}net_flags;


/*network high module structure*/
typedef struct 
{/*This is the network structure used for handler the connections and profiles associated by an application*/
  net_flags flags;                    /*<! Network control flags>*/
  uint8_t num_device_found;           /*<! This indicates the number of perispheral(s) discovered by the network >*/
  uint8_t num_device_connected;       /*<! This indicates the number of perispheral(s) success connected to the network >*/
  uint8_t num_device_serv_discovery;  /*<! This indicates the number of peer device(s) that had been scanned successfully their services and characteristic>*/
  dv_state_t device_cstatus;          /*<! Status of the device in this network >*/
  struct timer time_alive;            /*!< Time remaining before to consider the connection  or device(s)  lost >*/
  #ifdef MULTINODE                    
  connection_t mMSConnection[EXPECTED_NODES]; /*<! Here one connection per peer-node management by the application >*/
  #else
  connection_t mMSConnection;         /*<! Not multinode then only a single connection is possible. >*/
  #endif
}network_t;

/*****************************EVENT HANDLER STRUCTURES**********************************************/
typedef struct { /*this is the structured used by the event handler for specified an event*/
  tClockTime ISR_timestamp;
  uint16_t event_type;
  void * evt_data;
}event_t;
/***************************************************************************************************/



#endif /* NET_BLE_H*/