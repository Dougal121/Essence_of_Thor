


void fertigation_min (void) {
int i , j , k , v , jj ;
uint8_t fbit , board ;

  for ( j = 0 ; j < MAX_FERT ; j++){
//    if ((efert[j].AType & 0x90 ) != 0 ) {
      vfert[j].bRun = false ;
      vfert[j].Flowrate = 0 ;
//    }
  }
  k = dayOfWeek(now()) ;
  for ( i = 0 ; i < MAX_VALVE ; i++ ) {                                                                  // for each valve work what fertigation units are on
    if (( vvalve[i].lATTG > 1 ) || ( vvalve[i].lTTG > 1 ) || ((( evalve[i].TypeMaster & 0x40 ) != 0 ) && ( vvalve[i].iFB != 0 ))  ) {  // cut out one minute short
       if ( evalve[i].Fertigate != 0 ){                                                                  // check if switched on first
          for ( j = 0 ; j < MAX_FERT ; j++){
            if (((evalve[i].Fertigate & ( 0x01 << j )) != 0 ) && ( efert[j].CurrentQty > 0 )) {          // check which fertigation channels are active
              if ((( efert[j].DaysEnable & ( 0x01 << (k-1) )) != 0  ) && (( efert[j].DaysEnable & ( 0x80 )) != 0  )) {   // check if enabled for these days
                vfert[j].Flowrate += evalve[i].Flowrate ;                                                // work out the total flow rate 
                vfert[j].bRun = true ;    
                if ((efert[j].AType & 0xC0 ) != 0 ) {                                                    // if a chemical valve see what channel has the pump and keep it enabled
                  if ( MAX_FERT == 16 ){
                    v = ( efert[j].AType & 0x0f ) ;                                                        // 16 outputs (god help us!!! - eh) 
                  }else{
                    v = ( efert[j].AType & 0x07 ) ;                                                        // 8 outputs                       
                  }
//                  if ( v != 0 ) {                                                                        // cant use 0 as master solenoid ?? why
                    vfert[v].bRun = true ;                    
                    vfert[v].Flowrate += evalve[i].Flowrate ;                                            // work out the total flow rate 
//                  }                  
                }
              }
            }
          }     
       }
    }          
  }
  for ( j = 0 ; j < MAX_FERT ; j++){
    if (( efert[j].AType & 0x30 ) != 0  ) {
      board = ( efert[j].BoardBit & 0xf0 ) >> 4 ;
      fbit = ( efert[j].BoardBit & 0x0f ) ;
      if ( vfert[j].bRun ) {
        if (( efert[j].DaysEnable & 0x80 ) != 0 ){             // if it is enabled then start output
          if ( vfert[j].lTTG == 0 ) {                          // ie not in cycle
            if ( efert[j].BaseFlow > 0 ){
              vfert[j].lTTG = ( efert[j].OnTime * vfert[j].Flowrate / efert[j].BaseFlow ) + 1 ;
            }else{
              vfert[j].lTTG = efert[j].OnTime + 1 ;          
            }
            if (( efert[j].AType & 0x10 )== 0  ) {            
              efert[j].CurrentQty -= ( efert[j].PumpRate * vfert[j].lTTG ) ;  // only do if a normal pump
            }
            vfert[j].bOnOff = true ;  
            ActivateOutput(board , fbit , false , 0 ) ;
/*            for ( jj = 0 ; jj < MAX_FERT ; jj++){
              if (((efert[jj].AType & 0x40 ) != 0) && ( vfert[jj].bOnOff )&& (( efert[jj].AType & 0x0f ) == j )) {    
                efert[jj].CurrentQty -= ( efert[j].PumpRate * vfert[j].lTTG ) ;                  
                vfert[jj].bOnOff = true ;  
              }
            }*/  
          }
        }      
      }else{ // no run - It's sorta counter intuative but don't do anything to a master valve thats not running ... let it take it's natural course.
/*        vfert[j].lTTG = 0 ;
        if (vfert[j].bOnOff) { // if on turn off
          vfert[j].bOnOff = false ;
//          ActivateOutput(board , fbit , true , 0 ) ;  // active low so off is TRUE                                
        }  
        vfert[j].bRun = false ;*/
      } 
    }
  }
}


int fertigation_sec (void) {
int i , k , j ;
float ot  ;
uint8_t fbit , board ;
bool bFValve = false ;
bool bWValve = false ;
  k = 0 ;
  for ( i = 0 ; i < MAX_FERT ; i++){
    if ((efert[i].AType & 0x40 ) != 0  ) {  //  chemical valve coil    
      if ( vfert[i].bRun ) {
        bFValve = true ;
      }
    }
  }
  for ( i = 0 ; i < MAX_FERT ; i++){
    if ((efert[i].AType & 0x80 ) != 0 ) {  // wash valve coil   
      if ( bFValve ) {
        vfert[i].lTTG = efert[i].OffTime ;  // this is a arm sequence - ready to flush
      }else{
        if ( vfert[i].lTTG > 0 ){     // wait till there is a TTG and the other solinoids are all off
          bWValve = true ;            // we are in flush mode  
        }
      }
    }
  }

  for ( i = 0 ; i < MAX_FERT ; i++){
    board = ( efert[i].BoardBit & 0xf0 ) >> 4 ;
    fbit = ( efert[i].BoardBit & 0x0f ) ;
    switch ( ( efert[i].AType >> 4 ) ) {
      case 4: // Chemical Supply valve  
          if ( vfert[i].bRun ){  // if hasa flow etc turn on else off
            ActivateOutput(board , fbit , false , 0 ) ; // on active low  
            vfert[i].bOnOff = true ;
          }else{
            ActivateOutput(board , fbit , true , 0 ) ;  // off active high
            vfert[i].bOnOff = false ;
          }
      break;
      case 8: // wash water valve
        if ( bWValve ){     // wait till there is a TTG and the other solinoids are all off
          vfert[i].lTTG-- ;
          ActivateOutput(board , fbit , false , 0 ) ;   // ON active high (flipped)
          vfert[i].bOnOff = true ;
        }else{
          ActivateOutput(board , fbit , true , 0 ) ;     // off       
          vfert[i].bOnOff = false ;
        }
      break;
      case 2:                                     // 0x02   master pump 
        if ( bWValve ){                           // run on if wash sequence is activated
          ActivateOutput(board , fbit , false , 0 ) ;
          vfert[i].bOnOff = true ;
          vfert[i].lTTG = 2 ;                     // just hold it in a wee bit more than a tad
        }                                         // fall through don't break
      default:                                    // 0x01   garden variety proportioning pump
        if ( vfert[i].lTTG > 0 ){
          vfert[i].lTTG-- ;
        }
        if ( vfert[i].lTTG == 0 ){
          if ( vfert[i].bOnOff ){  // on so turn off
            ActivateOutput(board , fbit , true , 0 ) ;     // OFF        
            vfert[i].bOnOff = false ;
            if ( efert[i].BaseFlow > 0 ){
              vfert[i].lTTG = efert[i].OffTime ; // 
            }else{
              vfert[i].lTTG = efert[i].OffTime ;          
            }
          }else{   // off so turn on maybe
            if ( vfert[i].bRun ) {  // if hasa flow etc turn on else off
              if (( efert[i].BaseFlow > 0 ) && ( vfert[i].Flowrate >= 1  )){
                ot = efert[i].OnTime * vfert[i].Flowrate / efert[i].BaseFlow ;
              }else{
                ot = efert[i].OnTime ;          
              }
              if ( ot < 1 ){
                ot = 1 ;       
              }  
              if (( efert[i].AType & 0x10 ) != 0){   // normal pump
                efert[i].CurrentQty -= ( efert[i].PumpRate * ot ) ; // dont decrement the tank if a master pump only do if normal pump
              }else{
                ot = 0 ; // start again for master pump and add up all the on times
                for ( j = 0 ; j < MAX_FERT ; j++){
                  if (((efert[j].AType & 0xc0 ) != 0) && ( vfert[j].bOnOff )&& (( efert[j].AType & 0x0f ) == i ) && ( efert[j].CurrentQty > 0 )) {    
                    if (( efert[i].BaseFlow > 0 ) && ( vfert[i].Flowrate >= 1  )){
                      ot += efert[j].OnTime * vfert[i].Flowrate / efert[i].BaseFlow ;
                    }else{
                      ot += efert[j].OnTime  ;          
                    }
                    efert[j].CurrentQty -= ( efert[i].PumpRate * ot ) ;   // need to decrement the  base solenoid/tank                
                  }
                }  
              } 
              vfert[i].lTTG = ot ;       
              if ( ot > 0 ) {   
                vfert[i].bOnOff = true ;
                ActivateOutput(board , fbit , false , 0 ) ;
              }
            }   // bRun
          }
        }       // lttg = 0
        if ( vfert[i].bOnOff ) {
          k++ ;
        }
      break ;    
    }
  }
  return(k) ;
}

