// Script: wait until get "S" message from MCU then wait 1s,
//         this script is intend to check counter from TIMER interrupt
TIMEOUT(100000, log.log("last message: " + msg + "\n"));

// Select mote
mote = sim.getMoteWithID(1);
// Find variable by name
_var = mote.getMemory().getSymbolMap().get("OSTickCounter");
log.log(_var.name+" = "+_var.addr+"["+_var.size+"]\n");

// Start counting for 1 second
while(1){
  GENERATE_MSG(1000, "DONE");
  YIELD_THEN_WAIT_UNTIL(msg.equals("DONE"));

// Get value and transform it to number
var value = mote.getMemory().getMemorySegment(_var.addr, _var.size);
var number = 0;
for(aux=0;aux<value.length;aux++){
    number += (value[aux]&0xFF)<<(8*aux);
}
log.log("["+time+"]"+_var.name+" = "+number.toString()+"\n");
// value is a byte array with length value.length
}

log.testOK();