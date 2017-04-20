//import Java Package to JavaScript
 importPackage(java.io);

 TIMEOUT(99999999); // Set to infinity (disable timeout) (~1.15days)
 
 // Get simulation type (DEFAULT, FWR_ACK, FIN_BUFF)
 YIELD_THEN_WAIT_UNTIL(msg.equals("UDP server started"));
 YIELD(); // Get next message that contains CONTIKI_CONF_TYPE
 var sim_type = msg;

 // Set simulation parameters
 var sim_time = 30*1000; // Simulation time h*m*s*ms
 var sim_runs = 10;         // Simulation runs, how many time it'll run
 
 // Transform Date in YYYYMMDDHHmm
 Date.prototype.YYYYMMDDHHMM = function () {
        var yyyy = this.getFullYear().toString();
        var MM = pad(this.getMonth() + 1,2);
        var dd = pad(this.getDate(), 2);
        var hh = pad(this.getHours(), 2);
        var mm = pad(this.getMinutes(), 2);
        return yyyy + MM + dd+  hh + mm;
    };
 // String parser
 function pad(number, length) {
        var str = '' + number;
        while (str.length < length) {
            str = '0' + str;
        }
        return str;
    }
    
 // This will get date YYYYMMDDHHMM
 var today = new Date().YYYYMMDDHHMM();

 // Open log_<sim>.txt for writing.
 // BTW: FileWriter seems to be buffered.
 var sim_title = sim.getTitle();
 var output = new FileWriter("/home/user/logs/log_"+sim_title+"_"+sim_type+"_"+today+".csv");

 // Creater header
 /*output.write("#;IP.D;IP.R;IP.S;IP.F;"+
              "UDP.D;UDP.R;UDP.S;UDP.C;"+
              "TCP.D;TCP.R;TCP.S;TCP.C;"+
              "ICMP.D;ICMP.R;ICMP.S;ICMP.C;"+
              "Rime.ACKRX;Rime.ACKTX;Rime.RX;Rime.TX\n");*/

 // Number of columns except the node number, used it to create a sum for xls
 // var number_of_columns = 20;

 // Wait for simulation time (20min = 120000)
 GENERATE_MSG(sim_time, "sim:sleep");
 YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));

// mote.getInterfaces().getButton().clickButton();
 motes = sim.getMotes();
 for(var i = 0; i < motes.length; i++){
     motes[i].getInterfaces().getButton().clickButton();

     // If it's node 1, get header
     if(i == 0){
         // Get header
         YIELD_THEN_WAIT_UNTIL(msg.equals("Printing Header"));
         YIELD();
         output.write(msg+"\n"); // Got header
         log.log("[");
     }
     // Wait for the msg
     YIELD();
	 //Write to file.
     output.write(msg + "\n");

	 log.log(" *");
 }
 log.log(" ]\n");
 
 // Get footer (bottom)
 YIELD();
 output.write(msg + "\n");

 // Ouput script total sum (last line) for table formats
 /*output.write("SUM");
 var column = 'B';
 for(var i=0; i < number_of_columns; i++){
     // Skip first row (should be header)
     output.write(";=SUM("+column+"2:"+column+""+(motes.length+1)+")");
     // Increment column
     column = String.fromCharCode(column.charCodeAt(0) + 1);
 }
 output.write("\n");
*/
 output.close(); // Save file
 log.testOK();   // End script with success
