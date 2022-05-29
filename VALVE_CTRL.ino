int ProgramStatus(int Valve){
int iRet = 0 ;  
  for (int i = 0  ; i < MAX_PROGRAM ; i++ ){
    if (( vp[Valve].p[i].wdays & 0x80) != 0 ){  // program enables
      iRet |= 0x01 ;
      if (( vp[Valve].p[i].runtime & 0xffff) == 0){       // has zero runtime
        iRet |= 0x02 ;
      }
      if ((( vp[Valve].p[i].wdays & 0x7f )== 0 )) {
        iRet |= 0x04 ;                                   // enabled but no days selected        
      }
    }
  }
  return(iRet);
}

void UpdateATTG(void){
int k ;  
time_t start_t , finish_t ;
time_t now_t ;
time_t start_day_t ;
int dhour , dmin ;

   now_t = now() ;
   for ( int i = 0 ; i < MAX_VALVE ; i++ ) {       // for each valve
     if (((evalve[i].Valve & 0x80) == 0) || ( !evalve[i].bEnable )) {   // if enabled and uplink then dont reset
      vvalve[i].lATTG = 0 ;                         // set to zero -- i.e switch off
     }
     if ((( year() > 2000 ) && ( now() > ghks.AutoOff_t )) || ((evalve[i].TypeMaster & 0x200) != 0)){                     // dont do this if time is like ... BULLSHIT --switch off if time is crap (allow manual mode only + Always one Valves
        for ( int p = 0 ; p < MAX_PROGRAM ; p++ ) {
          dhour = vp[i].p[p].starttime / 100 ;
          dmin = vp[i].p[p].starttime % 100 ;
          for ( int d = 0 ; d < 7 ; d++ ) {                               // on each prgram check each day
            if (((vp[i].p[p].wdays & (0x01 << d )) != 0 ) && (vp[i].p[p].wdays & 0x80)) {                // if this day is enabled AND the master is enabled
              start_day_t = previousSunday(now_t) + ((d)*SECS_PER_DAY ) ;
              start_t = start_day_t + ( dmin * 60 ) + ( dhour * 3600 ) ;
              finish_t = start_t + ((vp[i].p[p].runtime ) * 60 ) ;        // finish in seconds is the start + minutes*60
              if (( now_t < finish_t ) && (now_t > start_t)){             // have a hit the valve should be on
                if (( evalve[i].bEnable )&&(( evalve[i].Valve & 0x80 ) == 0 )){                                 // check the valve is also switched on.... 
                  if ((evalve[i].Valve & 0x80) == 0){                       // must be not be in rx/uplink mode  
                    vvalve[i].lATTG = (finish_t - now_t + 58) / 60 ;        // display the time in minutes - the 58 is so we dont miss the last minute
                  }
                }
              }
            }
          }          
        }
     }
   }
}

void UpdateATTGNew(void){
int k ;  
time_t start_t , finish_t ;
time_t now_t ;
time_t start_day_t ;
int dhour , dmin ;

   now_t = now() ;
   for ( int i = 0 ; i < MAX_VALVE ; i++ ) {                                                 // for each valve
    if (((evalve[i].Valve & 0x80) == 0) || ( !evalve[i].bEnable )){
      vvalve[i].lATTG = 0 ;                                                                   // set to zero -- i.e switch off
    }
   } 
//   if (( year() > 2018 ) && ( now() > ghks.AutoOff_t )){  
   for ( int p = 0 ; p < MAX_PROGRAM_HEADER ; p++ ) {                                        // ie 0...8
      for ( int i = 0 ; i < MAX_STARTS ; i++ ) {                                             // for each start 0..4
        if ( pn.ph[p].StartTime[i] >= 0 ) {
          dhour = pn.ph[p].StartTime[i] / 100 ;                                                // MAX_STARTS
          dmin = pn.ph[p].StartTime[i] % 100 ;
          for ( int d = 0 ; d < 7 ; d++ ) {                                                    // on each prgram check each day
            if ((( pn.ph[p].WDays & (0x01 << d )) != 0 ) && (( pn.ph[p].WDays & 0x80) != 0 )) {   // if this day is enabled AND the master is enabled
              start_day_t = previousSunday(now_t) + ((d)*SECS_PER_DAY ) ;   // 
              start_t = start_day_t + ( dmin * 60 ) + ( dhour * 3600 ) ;
              for ( int j = 0 ; j < MAX_SHIFTS ; j++ ) {                                       // Scan all the shifts and set the valve if on 
                if (( pn.sh[j].Program  ==  p ) && ( pn.sh[j].RunTime  != 0 )){                // if the shift is part of the program
                  finish_t = start_t + ((pn.sh[j].RunTime ) * 60 ) ;            // finish in seconds is the start + minutes*60                
                  if (( now_t < finish_t ) && (now_t > start_t)){               // have a hit the valve should be on
                    for ( int v = 0 ; v < MAX_VALVE ; v++ ) {                   // for each valve
                      if (( evalve[v].bEnable )&&(( evalve[v].Valve & 0x80 ) == 0 )){                                 // check the valve is also switched on.... 
                        if (((  pn.sh[j].ValveNo & (0x01 << v )) != 0 )  ) {    // if this valve is selected in the shift activate it
                          if ((evalve[v].Valve & 0x80) == 0){                       // must be not be in rx/uplink mode  
                            if (( year() > 2000 ) && (( now() > ghks.AutoOff_t ) || ((evalve[v].TypeMaster & 0x200) != 0))){     // dont do this if the time is like ... BULLSHIT -- Don't switch on if time is crap (allow manual mode only or Always on option)                           
                              vvalve[v].lATTG = (finish_t - now_t + 58) / 60 ;      // display the time in minutes - the 58 is so we dont miss the last minute
                            }
                          }
                        }
                      } 
                    }
                  }else{
/*                      if ((now_t < start_t ) ){
                      for ( int v = 0 ; v < MAX_VALVE ; v++ ) {                   // for each valve
                        if (( evalve[v].bEnable ) && (vvalve[v].lATTG>0)){        // check the valve is also switched on.... 
                          vvalve[v].lATTG += pn.sh[j].RunTime ;
                        }
                      }
                    }  */
                  }                                                             // if time check
                  start_t = finish_t ;                                          // move forward to scan through all the shifts 
                }        
              }                                                                 // maxshifts for loop
            }
          }
        }          
      }     // for each start
   }
//   }
}






