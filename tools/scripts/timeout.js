// Script: wait until get "S" message from MCU then wait 1s,
//         this script is intend to check counter from TIMER interrupt
TIMEOUT(100000, log.log("last message: " + msg + "\n"));

WAIT_UNTIL(msg.equals("S"));

  GENERATE_MSG(1000, "DONE");
  YIELD_THEN_WAIT_UNTIL(msg.equals("DONE"));
  
log.testOK();