// Script: wait until get "S" message from MCU then wait 1s,
//         this script is intend to check counter from TIMER interrupt
TIMEOUT(100000, log.log("last message: " + msg + "\n"));

WAIT_UNTIL(msg.equals("S"));
// Program starts when led is on

counter = 0;
mote = sim.getMoteWithID(1);
led=false;

// Start counting for 1 second
GENERATE_MSG(1000, "DONE");
WAIT_UNTIL(msg.equals("DONE"));

// Find variable by name
teste = mote.getMemory().getSymbolMap();
debugdco = teste.get("debugdco_counter");
log.log(debugdco.name+" = "+debugdco.addr+"["+debugdco.size+"]\n");

// Get value and transform it to number
var value = mote.getMemory().getMemorySegment(debugdco.addr, debugdco.size); // short int read
var dco=0;
for(aux=0;aux<value.length;aux++){
    dco += (value[aux]&0xFF)<<(8*aux);
}
dco=dco*1000;
log.log("DCO = "+dco.toString()+"\n");
// value is a byte array with length value.length


log.testOK();