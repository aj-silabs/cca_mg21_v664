/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"
#include "../../platform/emdrv/nvm3/inc/nvm3.h"
#include "../../platform/radio/rail_lib/common/rail.h"

#include EMBER_AF_API_NETWORK_CREATOR
#include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_TARGET
//#include EMBER_AF_API_ZLL_PROFILE

#define LIGHT_ENDPOINT (1)
#define RSSI_OFFSET -11

EmberEventControl commissioningLedEventControl;
EmberEventControl findingAndBindingEventControl;


void commissioningLedEventHandler(void)
{
  emberEventControlSetInactive(commissioningLedEventControl);

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    uint16_t identifyTime;
    emberAfReadServerAttribute(LIGHT_ENDPOINT,
                               ZCL_IDENTIFY_CLUSTER_ID,
                               ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                               (uint8_t *)&identifyTime,
                               sizeof(identifyTime));
    if (identifyTime > 0) {
      halToggleLed(COMMISSIONING_STATUS_LED);
      emberEventControlSetDelayMS(commissioningLedEventControl,
                                  LED_BLINK_PERIOD_MS << 1);
    } else {
      halSetLed(COMMISSIONING_STATUS_LED);
    }
  } else {
    emDebugSendVuartMessage("Clear CCA counters\n",20);
    emberAfPluginCountersClear();
    EmberStatus status = emberAfPluginNetworkSteeringStart();
    emberAfCorePrintln("%p network %p: 0x%X", "Join", "start", status);
  }
}



#define MIN_CHANNEL 11
#define MAX_CHANNEL 27

/*
void commissioningLedEventHandler()
{
   
    RAIL_Handle_t handle;
    uint8_t current_channel;
    uint8_t channel;
     
    current_channel = emberAfGetRadioChannel();
    handle = emberGetRailHandle();
    
    emberAfCorePrintln("***********************************************");
    
    for(channel=MIN_CHANNEL;channel<MAX_CHANNEL;channel++){
          emberSetRadioChannel(channel);
          emberAfCorePrintln("channel:%d, RSSI:%d",channel,RAIL_GetRssi(handle,true));          
    }
    
   
    emberAfCorePrintln("***********************************************");
    emberSetRadioChannel(current_channel);
    
    
    
    emberEventControlSetDelayMS(commissioningLedEventControl,
                                  LED_BLINK_PERIOD_MS << 1);
    
}

*/

void findingAndBindingEventHandler()
{
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberEventControlSetInactive(findingAndBindingEventControl);
    emberAfCorePrintln("Find and bind target start: 0x%X",
                       emberAfPluginFindAndBindTargetStart(LIGHT_ENDPOINT));
  }
}

void adjustRssiOffset(int8_t offset)
{
  RAIL_Handle_t rail_handle = NULL;
  RAIL_RadioState_t state;
  RAIL_Status_t status = RAIL_STATUS_INVALID_PARAMETER;

  rail_handle = emberGetRailHandle();
  
//  RAIL_Idle(rail_handle,RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS,1);
  
  state = RAIL_GetRadioState(rail_handle);
  
  switch(state){
      case RAIL_RF_STATE_INACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_INACTIVE");
        break;
      case RAIL_RF_STATE_ACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_ACTIVE");
        break;
      case RAIL_RF_STATE_RX:
        emberAfCorePrintln("RAIL_RF_STATE_RX");
        break;
        
      case RAIL_RF_STATE_TX:
        emberAfCorePrintln("RAIL_RF_STATE_TX");
        break;
        
      
      case RAIL_RF_STATE_RX_ACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_RX_ACTIVE");
        break;
        
      case RAIL_RF_STATE_TX_ACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_TX_ACTIVE");
        break;
        
      default:
           emberAfCorePrintln("Unknown state");
           break;
  }

  if (rail_handle != NULL) {
      status = RAIL_SetRssiOffset(rail_handle, offset);
 
  
      if (status == RAIL_STATUS_NO_ERROR ) {
           emberAfCorePrintln("Set RSSI offset %d dBm error status %d.", offset, status);
      }else {
           emberAfCorePrintln("Get RAIL handle error.");
      }
   }
  
/*  

  RAIL_Idle(rail_handle,RAIL_IDLE,1);
  state = RAIL_GetRadioState(rail_handle);
  
  switch(state){
      case RAIL_RF_STATE_INACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_INACTIVE");
        break;
      case RAIL_RF_STATE_ACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_ACTIVE");
        break;
      case RAIL_RF_STATE_RX:
        emberAfCorePrintln("RAIL_RF_STATE_RX");
        break;
        
      case RAIL_RF_STATE_TX:
        emberAfCorePrintln("RAIL_RF_STATE_TX");
        break;
        
      
      case RAIL_RF_STATE_RX_ACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_RX_ACTIVE");
        break;
        
      case RAIL_RF_STATE_TX_ACTIVE:
        emberAfCorePrintln("RAIL_RF_STATE_TX_ACTIVE");
        break;
        
      default:
           emberAfCorePrintln("Unknown state");
           break;
  }
*/  
  
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  // Note, the ZLL state is automatically updated by the stack and the plugin.
  if (status == EMBER_NETWORK_DOWN) {
    halClearLed(COMMISSIONING_STATUS_LED);
  } else if (status == EMBER_NETWORK_UP) {
    halSetLed(COMMISSIONING_STATUS_LED);
    emberEventControlSetActive(findingAndBindingEventControl);
  }

// This value is ignored by the framework.
  return false;
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */



void emberAfMainInitCallback(void)
{
  uint8_t len = 20;
  
  emDebugInit();
  emDebugSendVuartMessage("Stack Init...\n",len);
  emberEventControlSetActive(commissioningLedEventControl);
  adjustRssiOffset(RSSI_OFFSET);
  
}

char *HEX_STRING = "0123456789ABCDEF";

char  result[20] = {0x20};

char * int16_to_string(uint16_t n)
{
    uint8_t i;
    uint16_t s;
    
    
//    n = 0x1234;
    
    
    for(i=0;i<4;i++){
      s = (0x000f) & (n >> (4 *(3-i)));
      
      emberAfCorePrintln("s:%d",s);
      
      result[i] = HEX_STRING[s];
    }
    
//    emberAfCorePrintln("result:%s",(char *) result);
    return result;
    
}

extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];

void put_cca_to_trace()
{
  uint8_t i;
  char *r;
  char prefix[] = "CCA counters:         ";
  
  r = int16_to_string(emberCounters[32]);
  emberAfCorePrintln("CCA couners:%d",emberCounters[32]);
  
  for(i=0;i<4;i++)
    prefix[13+i] = r[i];
  
  emDebugSendVuartMessage(prefix,25);

}

extern EmberAfPluginNetworkSteeringJoiningState emAfPluginNetworkSteeringState;
extern uint8_t emAfPluginNetworkSteeringTotalBeacons;
extern uint8_t emAfPluginNetworkSteeringJoinAttempts;

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  uint8_t i;
//  char *r;
//  char prefix[] = "CCA counters:         ";
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);

/*
    
  r = int16_to_string(emberCounters[32]);
  
  
  emberAfCorePrintln("CCA couners:%d",emberCounters[32]);
  
  for(i=0;i<4;i++)
    prefix[13+i] = r[i];
  
  emDebugSendVuartMessage(prefix,25);
  
*/
  put_cca_to_trace();
  
 
  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
        emAfPluginNetworkSteeringClearStoredPanIds();
        emAfPluginNetworkSteeringState = EMBER_AF_PLUGIN_NETWORK_STEERING_STATE_NONE;
        emAfPluginNetworkSteeringJoinAttempts = 0;
        emAfPluginNetworkSteeringTotalBeacons = 0;
//        emberEventControlSetDelayQS(commissioningLedEventControl,20);
        emberAfCorePrintln("%p network %p: 0x%X", "Join", "start", status);
  }

/*
  if (status != EMBER_SUCCESS) {
    // Initialize our ZLL security now so that we are ready to be a touchlink
    // target at any point.
    status = emberAfZllSetInitialSecurityState();
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error: cannot initialize ZLL security: 0x%X", status);
    }
*/
//    status = emberAfPluginNetworkCreatorStart(false); // distributed
//    emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);

}

/** @brief Complete
 *
 * This callback notifies the user that the network creation process has
 * completed successfully.
 *
 * @param network The network that the network creator plugin successfully
 * formed. Ver.: always
 * @param usedSecondaryChannels Whether or not the network creator wants to
 * form a network on the secondary channels Ver.: always
 */
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Form distributed",
                     "complete",
                     EMBER_SUCCESS);
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  emberAfOnOffClusterServerAttributeChangedCallback(endpoint,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID);
}

/** @brief Server Attribute Changed
 *
 * On/off cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    bool onOff;
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (uint8_t *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        halSetLed(ON_OFF_LIGHT_LED);
      } else {
        halClearLed(ON_OFF_LIGHT_LED);
      }
    }
  }
}

/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */
void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  if (state == BUTTON_RELEASED) {
    emberEventControlSetActive(findingAndBindingEventControl);
  }
}

/** @brief Message Sent
 *
 * This function is called by the application framework from the message sent
 * handler, when it is informed by the stack regarding the message sent status.
 * All of the values passed to the emberMessageSentHandler are passed on to this
 * callback. This provides an opportunity for the application to verify that its
 * message has been sent successfully and take the appropriate action. This
 * callback should return a bool value of true or false. A value of true
 * indicates that the message sent notification has been handled and should not
 * be handled by the application framework.
 *
 * @param type   Ver.: always
 * @param indexOrDestination   Ver.: always
 * @param apsFrame   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 * @param status   Ver.: always
 */
boolean emberAfMessageSentCallback(EmberOutgoingMessageType type,
                                   int16u indexOrDestination,
                                   EmberApsFrame* apsFrame,
                                   int16u msgLen,
                                   int8u* message,
                                   EmberStatus status)
{
   put_cca_to_trace();

   return false;
}

extern nvm3_Handle_t *nvm3_defaultHandle;

void getObjectCounts()
{
  emberAfCorePrintln("total objects in NVM3:%d",nvm3_countObjects(nvm3_defaultHandle));
}

void getKeyList()
{
  uint32_t keys[58];
  uint16_t num;
  
  nvm3_enumObjects(nvm3_defaultHandle,keys,232,NVM3_KEY_MIN,NVM3_KEY_MAX);
  
  for(num=0;num<58;num++)
    emberAfCorePrintln("key:%4x",keys[num]);
}

uint8_t * watch(uint8_t *value)
{
    return value;
}

void getObjectByKey()
{
  uint8_t  *value=NULL;
  uint32_t key;
  uint32_t object;
  uint32_t type;
  size_t len;
  
  uint8_t num;
  
  key = emberUnsignedCommandArgument(0);
  
  emberAfCorePrintln("key:%4x",key);
  
  nvm3_getObjectInfo(nvm3_defaultHandle,key,&type,&len);
  
  emberAfCorePrintln("object length:%2x",len);
  
  if (len > 0){
    
    value = malloc(len);
    
    watch(value);
  
    if (value !=  NULL){
    
       
        nvm3_readData(nvm3_defaultHandle,key,value,len);
  
        for(num=0;num<len;num++)
            emberAfCorePrintln("object:%x",value[num]);
    
        free(value);       
    }
  }
 }

void getCcaThreshold()
{
    int8_t signal;
    
    signal = emRadioGetEdCcaThreshold();
    emberAfCorePrintln("Threshold:%d\n",signal);
}

extern RAIL_Handle_t emPhyRailHandle;

void setCcaThreshold()
{
    int8_t signal;
    RAIL_Handle_t handle;
    
    signal = emberSignedCommandArgument(0);
    
//  emRadioSetEdCcaThreshold(signal);
    
    handle = emberGetRailHandle();
    
//    handle = emPhyRailHandle;
    
    emberAfCorePrintln("Handle:%d",(uint32_t)handle);
    
//    RAIL_SetCcaThreshold(handle,signal);
    emRadioSetEdCcaThreshold(signal);
}

void getRailOffsetThreshold()
{
    RAIL_Handle_t handle;
    handle = emberGetRailHandle();   
    emberAfCorePrintln("RSSI offset:%d",RAIL_GetRssiOffset(handle));
}

void setRssiOffsetThreshold(void)
{
  int8_t offset;
  RAIL_Handle_t handle;
  
  offset = emberUnsignedCommandArgument(0);
  handle = emberGetRailHandle();
  
  adjustRssiOffset(offset);
}

void getRssi()
{
    RAIL_Handle_t handle;
    handle = emberGetRailHandle();   
    emberAfCorePrintln("RSSI offset:%d",RAIL_GetRssi(handle,true));
}

EmberCommandEntry emberAfCustomCommands[] = {
#ifdef EMBER_AF_ENABLE_CUSTOM_COMMANDS
  
  emberCommandEntryAction("get-object-counts",
                          getObjectCounts,
                          "",
                          "get object number from NVM3"),
  emberCommandEntryAction("get-object-by-key",
                          getObjectByKey,
                          "w",
                          "get object by key from NVM3"),
  emberCommandEntryAction("get-key-list",
                          getKeyList,
                          "w",
                          "get key list in NVM3"),
  
  emberCommandEntryAction("set-cca-threshold",
                          setCcaThreshold,
                          "s",
                          "get cca threshold"),
  
  emberCommandEntryAction("get-cca-threshold",
                          getCcaThreshold,
                          "",
                          "get cca threshold"),
  
  emberCommandEntryAction("get-Rail-Offset-threshold",
                          getRailOffsetThreshold,
                          "",
                          "get rssi offset threshold"),
  
  emberCommandEntryAction("set-Rail-Offset-threshold",
                          setRssiOffsetThreshold,
                          "s",
                          "set rssi offset"),
  
  emberCommandEntryAction("get-Rssi",
                          getRssi,
                          "",
                          "get Rssi"),
  
  #endif //ENABLE_CUSTOM_COMMANDS
  emberCommandEntryTerminator(),
};
