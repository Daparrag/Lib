/*event handler file */

/*the idea of this module is to generalize the ble events,
 thus they could be access for any other process in the system*/

/*multiqueue associate to specific type events for efficient resource management*/

/*allows management the queue according to  priorities for high efficient event management*/


#include <eventhandler.h>
#include <list.h>

/*****************************macros**********************************************/
#define COPY_EVENT(dest,source) memcpy(dest,source,sizeof((source)))

/****************** Variable Declaration **************************/
LIST(event_queue); /*definition of the network queue*/
struct event_entry _events [EVENT_QUEUE_SIZE];

event_t _event;
uint8_t start_slot;
uint8_t end_slot;
static const uint8_t max_slots = EVENT_QUEUE_SIZE-1; 

/********************static functions*******************/
static uint8_t HCI_Get_Event_Queue_Size_CB(void);
/********************************************************/



/**
  * @brief  Used for indicate that there are pending events to process .
  * @param void.
  * @retval uint8_t: return 1 if there are pending events to process otherwhise 0
  */
  
uint8_t HCI_new_Event_CB(void){
  if(list_length(event_queue)) return 1;
  return 0;
}


/**
  * @brief  Used for initialized the event handler module.
  * @param none.
  * @retval none,
  */

void HCI_Init_Event_CB(void){
        start_slot = 0;
        end_slot = max_slots;
	list_init(event_queue);
}



/**
  * @brief  This function return the number of pending events .
  * @param none.
  * @retval uint8_t: return  the number of pending events
  */
uint8_t HCI_Get_Event_Queue_Size_CB(void)
{
  
  return list_length(event_queue);
}


/**
  * @brief  This function retreve status of the queeue: empty, full, normal .
  * @param none.
  * @retval uint8_t: Status of the event queue. 
  */

uint8_t HCI_Get_Event_Queue_status_CB(void){
  
    if (list_length(event_queue) == EVENT_QUEUE_SIZE-1) return QUEUE_FULL;
    if (list_length(event_queue) == 0)return QUEUE_EMPTY;
    return QUEUE_NORMAL;
}


/**
  * @brief return a free slot in the queeue for attach a new event.
  * @param none.
  * @retval uint8_t: slot index. 
  */

uint8_t HCI_Get_Entry_Index_CB(void)
{
/*insert control mechanism  to avoid overwrite slots which are not yet processed */  
  
  uint8_t free_slots;
  uint8_t queue_size = HCI_Get_Event_Queue_Size_CB();

  if(start_slot == end_slot){
    if(HCI_Get_Event_Queue_status_CB() != QUEUE_FULL)
    {
      free_slots = max_slots - queue_size;
        if(end_slot == max_slots){
          start_slot = 0; end_slot = free_slots;
        }else if(end_slot + free_slots > max_slots){
          start_slot = end_slot; end_slot = max_slots;
        }else {
          start_slot = end_slot; end_slot += free_slots;  
        }
      return start_slot;   
    }
       
  }
  
  return start_slot;
}


/**
  * @brief add and event in the index queue.
  * @param struct event_entry * entry : event structure
  * @retval none. 
  */

void HCI_add_Event_CB(struct event_entry * entry){
  
  list_add (event_queue, entry );
  start_slot+=1;
  
}

/**
  * @brief retreve an event form the event queue.
  * @param none
  * @retval event_t *: Pointer to an event . 
  */
event_t * HCI_Get_Event_CB(void){
  struct event_entry * top_entry;
  event_t * event;
  top_entry = (struct event_entry *) list_head(event_queue);
  event = &top_entry->event_val;

  return event;
  
    
}

/**
  * @brief Remove the las event processed.
  * @param none
  * @retval none . 
  */
void HCI_clean_last_Event_CB(void){
list_pop (event_queue);
}

/**
  * @brief event handler whitout queue.
  * @param void *pckt: pointer to an input packet
  * @retval none . 
  */
void HCI_Event_Handler_CB_(void *pckt){/*this version does not uses the event handler queue*/
  hci_uart_pckt *hci_pckt = pckt;
  hci_event_pckt * event_pckt = (hci_event_pckt*)hci_pckt->data;
  if(hci_pckt->type != HCI_EVENT_PKT)return;
  
  switch(event_pckt->evt){
    
    case EVT_DISCONN_COMPLETE:
	 	{
	 		
                  while(1);
	 		
	 	}
	 	break;

	 	case EVT_LE_META_EVENT:
	 	{
	 		  evt_le_meta_event *evt = (void *)event_pckt->data;
	 		   switch(evt->subevent)
	 		   {
	 		   	 case EVT_LE_CONN_COMPLETE:
	 		   	 {
                                   //PRINTDEBUG("EVT_LE_CONN_COMPLETE");
                                    evt_le_connection_complete *cc = (void *)evt->data;
                                    _event.event_type = EVT_LE_CONN_COMPLETE;
                                    _event.evt_data =cc;
                                     network_process(&_event);
                                   

	 		   	 }
	 		   	 break;

	 		   	 case EVT_LE_ADVERTISING_REPORT:
	 		   	 {
	 		   	 	
                                    le_advertising_info *pr = (le_advertising_info*) (((uint8_t*)evt->data)+1);
                                   _event.event_type = EVT_LE_ADVERTISING_REPORT;
                                    _event.evt_data = pr;
                                    network_process(&_event);
                                    // BSP_LED_On(LED2);
                                    // while(1);
	 		   	 }
	 		   	 break;
                                 
                                 case EVT_BLUE_GAP_DEVICE_FOUND:
                                   {
                                   /*IDB04A1*/
                                   }
                                   break;
	 		   }
	 	}
	 	break;
                case EVT_VENDOR:
                {
                  evt_blue_aci * blue_evt = (void*)event_pckt->data;
                    switch(blue_evt->ecode)
                    {
                        case EVT_BLUE_GATT_PROCEDURE_COMPLETE:
                          {
                            _event.event_type=EVT_BLUE_GATT_PROCEDURE_COMPLETE;
                            evt_gatt_procedure_complete * pr = (void*)blue_evt->data;
                              _event.evt_data = pr;
                              network_process(&_event);
                          }
                          break;
                      
                    
                    }
                    
                }
                break;
                
  }
}

/**
  * @brief event handler using queue.(does not works at the moment)
  * @param void *pckt: pointer to an input packet
  * @retval none . 
  */
void HCI_Event_Handler_CB(void *pckt){/*this is the version that uses the event queue (does not work)*/
uint8_t index_queue;
  hci_uart_pckt *hci_pckt = pckt;
  hci_event_pckt * event_pckt = (hci_event_pckt*)hci_pckt->data;
  index_queue = HCI_Get_Entry_Index_CB();

	if(hci_pckt->type != HCI_EVENT_PKT)return;
	if(HCI_Get_Event_Queue_status_CB() == QUEUE_FULL) return;

	switch(event_pckt->evt){
	 	case EVT_DISCONN_COMPLETE:
	 	{
	 		
	 		//COPY_EVENT (&_events[index_queue].event_val.event_data, event_pckt);
	 		_events[index_queue].event_val.event_type = EVT_DISCONN_COMPLETE;
	 		
	 	}
	 	break;

	 	case EVT_LE_META_EVENT:
	 	{
	 		  evt_le_meta_event *evt = (void *)event_pckt->data;
	 		   switch(evt->subevent)
	 		   {
	 		   	 case EVT_LE_CONN_COMPLETE:
	 		   	 {
	 		   	 	//COPY_EVENT (&_events[index_queue].event_val.event_data,event_pckt);
	 		   	 	_events[index_queue].event_val.event_type = EVT_LE_CONN_COMPLETE;
                                        HCI_add_Event_CB(&_events[index_queue]);
                                        

	 		   	 }
	 		   	 break;

	 		   	 case EVT_LE_ADVERTISING_REPORT:
	 		   	 {
	 		   	 	
                                 //   le_advertising_info *pr = (le_advertising_info*) (((uint8_t*)evt->data)+1);
                                 //   _events[index_queue].event_val.event_type = EVT_LE_ADVERTISING_REPORT;
                             
                                    // BSP_LED_On(LED2);
                                    // while(1);
	 		   	 }
	 		   	 break;
                                 
                                 case EVT_BLUE_GAP_DEVICE_FOUND:
                                   {
                                   /*IDB04A1*/
                                   }
                                   break;

	 		   

	 		   }
	 	}
	 	break;
	}

    

}



void HCI_Isr_Event_Handler_CB(){
  /*Here had been modified the HCI_Isr method to allows:
  *
  *1. timestamp for all imput packages.
  *2. allocate the input packets into the packet queue with priorities.
  *
  */

  tHciDataPacket * hciReadPacket = NULL;
  uint8_t data_len;
  tClockTime HCI_Isr_time = Clock_Time(); /*this is the current time in which the packet was transfered from the PoolQueue to the ReadRXqueue*/
  Clear_SPI_EXTI_Flag();

   while(BlueNRG_DataPresent()){ 
      if(list_is_empty (&hciReadPktPool) == FALSE){

         /* enqueueing a packet for read */
        list_remove_head (&hciReadPktPool, (tListNode **)&hciReadPacket);
        data_len = BlueNRG_SPI_Read_All(hciReadPacket->dataBuff, HCI_READ_PACKET_SIZE);
        if(data_len > 0){
          hciReadPacket->
          hciReadPacket->data_len = data_len;
          if(HCI_verify(hciReadPacket) == 0){
            hciReadPacket->Isr_timestamp = HCI_Isr_time;

            if(Packet_Get_Priority(hciReadPacket)!=0){
                /*insert packet according to the priority level */

            }else{
              /*if not priority insert in the tail*/
              list_insert_tail(&hciReadPktRxQueue,(tListNode *)hciReadPacket);
            }


          }else{
              /*this is a wrong packet(it is not an event_packet)*/
              /* Insert the packet back into the pool.*/
              list_insert_head(&hciReadPktPool, (tListNode *)hciReadPacket);
          }

        }else{
          /*this is a wrong packet (packet without length)*/
           /* Insert the packet back into the pool.*/
              list_insert_head(&hciReadPktPool, (tListNode *)hciReadPacket);
        }
   }else{
      // HCI Read Packet Pool is empty, wait for a free packet.
      Clear_SPI_EXTI_Flag();
      return;
   }

  Clear_SPI_EXTI_Flag();
  }
}



void HCI_Packet_Release_Event_Handler_CB(){
  tHciDataPacket * hciReadPacket = NULL;

Disable_SPI_IRQ();
   uint8_t list_empty = list_is_empty(&hciReadPktRxQueue);  
 
    if(list_empty == FALSE){    
      list_remove_head (&hciReadPktRxQueue, (tListNode **)&hciReadPacket);
      list_insert_tail(&hciReadPktPool, (tListNode *)hciReadPacket);    
    }else{
       /* Explicit call to HCI_Isr(), since it cannot be called by ISR if IRQ is kept high by
      BlueNRG. */    
      HCI_Isr();
    }

Enable_SPI_IRQ();    

}


void * HCI_Get_Event_Event_Handler_CB(){

   tHciDataPacket * hciReadPacket = NULL;
   Disable_SPI_IRQ();
    uint8_t list_empty = list_is_empty(&hciReadPktRxQueue);
    if(list_empty==FALSE){


      list_get_head (&hciReadPktRxQueue, (tListNode **)&hciReadPacket);/**/
      Enable_SPI_IRQ();
      hci_uart_pckt *hci_pckt = (hciReadPacket->dataBuff);
      hci_event_pckt * event_pckt = (hci_event_pckt*)(hci_pckt->dataBuff);
       
       if(hci_pckt->type != HCI_EVENT_PKT){
        /*BLUENRG only support event_packets*/
        return NULL;
        }

        switch(event_pckt->evt){

          case EVT_DISCONN_COMPLETE:
          {
                  /*stil is not defined*/
                  while(1);
      
          }
          break;

            case EVT_LE_META_EVENT:
            {

              evt_le_meta_event *evt = (void *)event_pckt->data;
              switch(evt->subevent){
                  case EVT_LE_CONN_COMPLETE:
                  {
                    evt_le_connection_complete *cc = (void *)evt->data;
                    _event.event_type = EVT_LE_CONN_COMPLETE;
                    _event.evt_data =cc;
                    _event.ISR_timestamp = hciReadPacket->Isr_timestamp;
                    return (void*)&_event;
                  }
                  break;
                  case EVT_LE_ADVERTISING_REPORT:
                  {
                    le_advertising_info *pr = (le_advertising_info*) (((uint8_t*)evt->data)+1);
                    _event.event_type = EVT_LE_ADVERTISING_REPORT;
                    _event.evt_data = pr;
                    _event.ISR_timestamp = hciReadPacket->Isr_timestamp;
                    return (void*)&_event;
                  }
                  break;


              }

            }
            break;
            case EVT_VENDOR:
            {
              evt_blue_aci * blue_evt = (void*)event_pckt->data;
              switch(blue_evt->ecode)
              {

                case EVT_BLUE_GATT_PROCEDURE_COMPLETE:
                {
                  evt_gatt_procedure_complete * pr = (void*)blue_evt->data;
                  _event.event_type=EVT_BLUE_GATT_PROCEDURE_COMPLETE;
                  _event.evt_data = pr;
                  _event.ISR_timestamp = hciReadPacket->Isr_timestamp;
                }
                break;
              }
            }
            break;
        }
    }else{
      Enable_SPI_IRQ();
    }

}
