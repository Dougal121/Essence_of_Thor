int filter_scan (void){
int i ,k ;
  for ( k = 0 ; k < MAX_FILTER ; k++){
    if ( efilter[k].Enable != 0 ) {
      vfilter[k].bOnOff = false ;   //reset them all
    }
  }
  for ( i = 0 ; i < MAX_VALVE ; i++ ) {             // for each valve work out which turn filter units on
    if (( evalve[i].Filter != 0 )&&( vvalve[i].bOnOff == true )){
      for ( k = 0 ; k < 8  ; k++ ) {                // for each valve work out which turn filter units on
        if (( evalve[i].Filter & ( 0x01 << k )) != 0 ){
          vfilter[k].bOnOff = true ;  // set them on if they need be active
        }
      }
    }
  }  
  for ( k = i = 0 ; i < MAX_FILTER ; i++){
    if ( vfilter[i].bOnOff ) {
      k++ ;   //count them
    }
  }
  return(k);
}

int filter_sec (void){
uint8_t fbit , board ;
int i ,k ;
bool bPol ;
  for ( i = 0 ; i < MAX_FILTER ; i++){
    if ( efilter[i].Enable != 0 ) {
      board = ( efilter[i].BoardBit[k] & 0xf0 ) >> 4 ;
      fbit = ( efilter[i].BoardBit[k] & 0xf0 ) ;
      if ((bPol & ( 0x01 << k )) != 0 ) {
        bPol = false ;
      }else{
        bPol = true ;
      }
      if ( vfilter[i].bOnOff ) {
        k = vfilter[i].NextCan ;
        if (( k > 7 )||( k < 0 )){
          k = 0 ;
        }
        vfilter[i].lTTG-- ;
        if ( vfilter[i].lTTG == 0 ) {
          if ( vfilter[i].bFlush ) {
//            IOEXP[board].write(fbit , bPol  );   // Off active HIGH
            ActivateOutput(board , fbit , bPol , 0 ) ;
            
            vfilter[i].lTTG = efilter[i].WashDelay ;
            vfilter[i].bFlush = false ;
            if ( ++vfilter[i].NextCan > efilter[i].Canisters ) {
              vfilter[i].NextCan = 0 ;
            }
          }else{
//            IOEXP[board].write(fbit , !bPol );   // On active LOW
            ActivateOutput(board , fbit , !bPol , 0 ) ;
            vfilter[i].lTTG = efilter[i].WashTime ;
            vfilter[i].bFlush = true ;
          }
        }
      }else{
//        IOEXP[board].write(fbit , bPol  );   // Off active HIGH    
        ActivateOutput(board , fbit , bPol , 0 ) ;
      }
    }
  }
  return(0);
}

