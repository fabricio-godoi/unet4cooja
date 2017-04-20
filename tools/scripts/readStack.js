// Script: wait until get "S" message from MCU then wait 1s,
//         this script is intend to check counter from TIMER interrupt
TIMEOUT(100000, log.log("last message: " + msg + "\n"));

// Select mote
mote = sim.getMoteWithID(1);
// Find variable by name
_var = mote.getMemory().getSymbolMap().get("STACK");
log.log(_var.name+" = "+_var.addr+"["+_var.size+"]\n");
_var_type = 4; // 32bits

// Start counting for 1 second
//while(1){
  GENERATE_MSG(1000, "DONE");
  YIELD_THEN_WAIT_UNTIL(msg.equals("DONE"));

// Get value and transform it to number
var value = mote.getMemory().getMemorySegment(_var.addr, _var.size);
var number = 0;
for(j=0;j<value.length;j++){
    if(j%_var_type==0){
        log.log(number.toString(16)+"\n");
        number = 0;
    }
    number += (value[j]&0xFF)<<(8*(j%_var_type));
}
log.log(number.toString(16)+"\n");
// value is a byte array with length value.length
//}


log.testOK();