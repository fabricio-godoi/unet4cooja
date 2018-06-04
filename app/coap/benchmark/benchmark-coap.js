// 
// \brief  Log mote statistics in csv file
// \author Fabrício Negrisolo de Godoi
// \date   2017-10-18
// \details
//   This script is performed in 4 steps:
// 1st - Definition of used variables and motes_statistics function, in which:
//       - motes_statistics is the variable that store all motes statistics,
//         "header" variable must have the same structure of motes_statistics;
//       - server_collection is a server variable informing the packets
//         received by each client;
//       - "header" must contains the exact format of the statistics
//         variable in the mote write in .csv;
//       - server information is got from UART parsing server messages in 
//         this exact format: "server: os_name network_type"
// 2nd - Simulation initialization (UART):
//       - Get server information and inform each mote the network size
// 3rd - Simulation running (UART):
//       - Wait the network stabilization time ...
//           ... Each client has a path to server + delay time (eg. 10s)
//       - Start all motes (clean statistics and start the processes) ...
//       - Wait each mote send "BMCD_DONE" after sending the number of packets
//         setup in bm_packets variable;
//       - After all motes sent "BMCD_DONE", stop all motes (statistcs and processes);
//       - Wait buffering clear time "buffer_clean_delay";
// 4th - Results collections (RAM):
//       - Read the motes memory collecting the statistics specified by
//         header directly from the memory;
// 5th - Run next simulation with new transmission rate:
//       - New transmission rate is calc by: "bm_interval-bm_interval_dec"
//       - Runs end when bm_interval is less than bm_min_interval;
//
// Obs.:
//  Serial at: cooja.interfaces.SerialPort.java
//  Memory at: cooja.mote.memory.MemoryInterface.java
//

//############################################################
// 
//  Benchmark for CoAP
// 

//############################################################
// 
//  Simulation parameters
// 
// TIMEOUT must be the first thing to set
TIMEOUT(86400000); // 24 hours timeout

// Get system information
if(!msg.startsWith("server:")){ YIELD_THEN_WAIT_UNTIL(msg.startsWith("server:"));}
var server_id = id;
var server_mote = mote;
var string = msg.replace("server: ","").split(" ");
var os_name = string[0];
var os_net = string[1]; //bm_nofmsgs = string[2];

// This is used to create the output log
var sim_title = sim.getTitle();
var log_path = "/home/user/logs";

// This will store all motes information (simplify the script)
var motes = sim.getMotes();

// After all motes get at least one route, wait to network settle
var bm_network_settle_time = 20000; // default: 15s

//! Do not need in CoAP
// This delay is intended to clear the network buffer (hop/hop)
var buffer_clean_delay = 60000; // wait ten seconds to ensure that the buffer is clear

//! Do not need in CoAP
// This delay is used to ensure that the previous simulation run has ended
var delay_between_runs = 10000; // wait two seconds to run another simulation

//Number of packets by each mote
var bm_packets = 100;	// value should not exceed 127

// Transmission start interval in ms
var bm_interval = 16000; // Use a value to start each mote in CoAP

//! Do not need in CoAP
//Transmission interval decrease value rate is ms
var bm_interval_dec = 3000; // Time doesn't matter in CoAP

//! Do not need in CoAP
// Minimum interval value acceptable in ms
var bm_min_interval = 1000; // Time doesn't matter in CoAP

// Statistcs variables
var motes_statistics = "UNET_NodeStat";
var server_collection = "bm_pkts_recv";

//Caculate total simulation time
var total_simulation_time = 0;
for(var k = bm_interval; k >= bm_min_interval; k-=bm_interval_dec){
	total_simulation_time += k*bm_packets + buffer_clean_delay + delay_between_runs;
}

// Print simulation parameters
log.log("==========================================\n");
log.log("Simulation Parameters\n");
log.log("Server ID: "+server_id+"\n");
log.log("Server OS: "+os_name+"\n");
log.log("OS Network: "+os_net+"\n");
log.log("Packts/mote: "+bm_packets+"\n");
log.log("Transmission interval: "+bm_interval+"ms\n");
log.log("Total simulation time expected: "+ms2time(total_simulation_time)+"\n");
log.log("==========================================\n");
//// TODO need to add simulation environment configuration to output file

//############################################################
//import Java Package to JavaScript
importPackage(java.io);

//############################################################
//
//  Auxiliary functions definitions
//
//Transform Date in YYYYMMDDHHmm
Date.prototype.YYYYMMDDHHMM = function () {
var yyyy = this.getFullYear().toString();
var MM = pad(this.getMonth() + 1,2);
var dd = pad(this.getDate(), 2);
var hh = pad(this.getHours(), 2);
var mm = pad(this.getMinutes(), 2);
return yyyy + MM + dd+  hh + mm;
};
//String parser
function pad(number, length) {
var str = '' + number;
while (str.length < length) {
    str = '0' + str;
}
return str;
}
//Parse time into mm:ss.mss
function tick2time(t){
	var milisec = Math.floor(t/1000)%1000;
	var seconds = Math.floor(t/(1000*1000))%60;
	var minutes = Math.floor(t/(1000*1000*60));
	if(minutes >= 60){
	     var hours = minutes/60;
	     minutes = minutes%60;
	     return ""+Math.floor(hours)+":"+("0"+minutes).slice(-2)+":"+("0"+seconds).slice(-2)+"."+("00"+milisec).slice(-3);
	}
	else return ""+("0"+minutes).slice(-2)+":"+("0"+seconds).slice(-2)+"."+("00"+milisec).slice(-3);
}
//Parse time into mm:ss.mss
function ms2time(t){
	var minutes = Math.floor(t/(1000*60));
	var seconds = Math.floor(t/(1000))%60;
	var milisec = t%1000;
	if(minutes >= 60){
	     var hours = minutes/60;
	     minutes = minutes%60;
	     return ""+Math.floor(hours)+":"+("0"+minutes).slice(-2)+":"+("0"+seconds).slice(-2)+"."+("00"+milisec).slice(-3);
	}
	else return ""+("0"+minutes).slice(-2)+":"+("0"+seconds).slice(-2)+"."+("00"+milisec).slice(-3);
}
//Parse an byte and put it as hex string
function byte2hex(byte) {
	var b2s = ["0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"];
return ""+(b2s[((byte>>4)&0x0f)])+(b2s[byte&0x0f]);
}
//Loading bar functions
function loading(){this.printed = 0;}
loading.prototype.start = function(){
	log.log("0%-------------------------------------------------100%\n");
	log.log("|<");
	printed = 0;
}
loading.prototype.load = function(percent){
	while(this.printed < 100*percent){
		log.log("-");
		this.printed+=100/50;
	}
}
loading.prototype.end = function(){
	log.log(">|\n");
}
//Parameters mote, name and byte size (1 - char, 2 - short, 4 - long ...)
//Return string array with values eg. "0;1;32;45;128;32;..."
function readVariable(_mote, _name, _bytes){
	var _return = "";
	try{
		// Find variable by name
		_var = _mote.getMemory().getSymbolMap().get(_name);
		
		// Get value and transform it to number
		var value = _mote.getMemory().getMemorySegment(_var.addr, _var.size);
		var i = 0, j = 0, hex;
		for(i=0;i<value.length;i+=_bytes){
			// Since values read from memory is signed (-128 to 127)
			// transform it into hex string and parse to be from 0 to 255
			hex = "";
			for(var m = _bytes-1; m >= 0; m--)
				hex += byte2hex(value[i+m]);
			_return += parseInt(hex,16)+";";
		}
	} catch(err) {
		log.log("\nVariable not found in mote id "+_mote.getID()+"\n");
		log.log(err.toString()+"\n");
		log.testFailed();
	}
	return _return.slice(0,-1); // remove last ';' before returning
}

//############################################################
//
//  Variables definitions
//

//This will get date YYYYMMDDHHMM
var today = new Date().YYYYMMDDHHMM();

// Log the route used in the simulation
var log_route = "timestamp;from;to\n";

//Creater header (this must be sync with the struct)
var header = "#;" +
		"pkts/client;" +
		"apptxed;" +
		"apprxed;" +
		"netapptx;" +
		"netapprx;" +
		"duplink;" +
		"routed;" +
		"dupnet;" +
		"routdrop;" +
		"txed;" +
		"rxed;" +
		"txfailed;" +
		"txnacked;" +
		"txmaxretries;" +
		"chkerr;" +
		"overbuf;" +
		"dropped;" +
		"corrupted;" +
		"radiotx;" +
		"radiorx;" +
		"radiocol;" +
		"radionack" +
		"\n";

// Benchmark output control commands, check C source code for more information
var BMC_START    = 1;
var BMC_STOP     = 1<<1;
var BMC_STATS    = 1<<2;
var BMC_RESET    = 1<<3;
var BMC_SHUTDOWN = 1<<4;
var BMC_GET      = 1<<5;
var BMC_SET      = 1<<6;
/// Note: last bit cannot be used with Javascript in a easy method
/// since 0x80 would transform the number in negative, the data
/// structure from js would transform it on a data larger than a byte
/// performing error when send by "writeArray"

var BMC_COMMAND = [];
var BCM_END     = 0x0a; // Ends with '\n'

//############################################################
//
//   Wait OS Startup
//

//Benchmark input control commands
//When received this message from every client node, the simulation can start 
var BM_WAIT_CLIENT = "BMCC_START";

/// Notify all clients with the network size
//Wait system wake up
GENERATE_MSG(1000, "sim:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));
// First, send the command to ensure at least a initial configure
BMC_COMMAND = [(0xFF & BMC_STOP | BMC_SET),// Command
               ((motes.length-1) & 0xFF),  // Number of motes
               (bm_packets & 0x7F),        // Number of packets
               ((bm_interval>>7)&0x7F),    // Interval MSB
               (bm_interval&0x7F),         // Interval LSB
               BCM_END];                   // End
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}
//############################################################
// 
//  Network stabilization
// 
log.log("Waiting network stabilization ["+(motes.length-1)+"] ...\n");

//Wait for every client mote to get ready
var load = new loading();
load.start();
var started_clients = 0;
do{
	YIELD_THEN_WAIT_UNTIL(msg.contains(BM_WAIT_CLIENT) || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg.split(" ")[1]+"\n";
	}
	else{
		started_clients++;
		load.load((started_clients)/(motes.length-1));
	}
}while(started_clients < (motes.length - 1));
load.end();

// Stabilization time
var startup_time = time;
log.log("Path creation time: "+tick2time(startup_time)+"\n");

// Wait ten seconds to be sure that the network is stable
GENERATE_MSG(bm_network_settle_time, "sim:sleep");
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg.split(" ")[1]+"\n";
	}
}while(!msg.equals("sim:sleep"));
log.log("Stabilization total time: "+tick2time(time)+"\n");
// Estimate time to finish
log.log("Simulation expected to finish at: "+tick2time(time+total_simulation_time*1000)+"\n");


//############################################################
//
//  Simulation start
//
do{

// Configure all motes with correctly interval and network size
BMC_COMMAND = [(0xFF & BMC_STOP | BMC_SET | BMC_RESET),  // Command
               ((motes.length-1) & 0xFF),                // Number of motes
               (bm_packets & 0x7F),                      // Number of packets
               ((bm_interval>>7)&0x7F),                  // Interval MSB
               (bm_interval&0x7F),                       // Interval LSB
               BCM_END];                                 // End
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}

log.log("==========================================\n");
log.log("Run expected to finish at: "+tick2time(time+bm_interval*bm_packets*1000)+"\n");

//Start all motes, server first
BMC_COMMAND = [(0xFF & BMC_START|BMC_STATS), BCM_END];
server_mote.getInterfaces().get("Serial").writeArray(BMC_COMMAND);
for(var k = 0; k < motes.length; k++){
	if(motes[k].getID()!=server_id)
		motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}
log.log("Simulation running with interval "+bm_interval+"...\n");

// Wait simulation time
var waiting = (motes.length - 1); // In CoAP wait only for the Server to receive all msgs
var sim_started = time;
do{
	YIELD_THEN_WAIT_UNTIL(msg.contains("BMCD_DONE") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg.split(" ")[1]+"\n";
	}
	else if(msg.contains("BMCD_DONE")) waiting--;
}while(waiting > 0);
log.log("All motes done sending messages...\n");

// Stop all motes messages, but continue the statistics
BMC_COMMAND = [(0xFF & BMC_STOP|BMC_STATS), BCM_END];
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}
log.log("Time: "+tick2time(time)+"\n");

// Wait buffer delay time
log.log("Waiting network buffer...\n");
log.log("Buffer expected to be clean at: "+tick2time(time+buffer_clean_delay*1000)+"\n");
GENERATE_MSG(buffer_clean_delay, "sim:sleep");
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg.split(" ")[1]+"\n";
	}
}while(!msg.equals("sim:sleep"));

// Stop all motes statistics
BMC_COMMAND = [(0xFF & BMC_STOP), BCM_END];
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}


//############################################################
//
//  Extract data from motes
//
log.log("Retrieving data from motes ["+motes.length+"] ...\n");

//Create output folder
var output_folder = log_path+"/"+os_name+"_coap/"+os_net+"_"+sim_title+"_"+today;
if(!new File(output_folder).exists())  new File(output_folder).mkdirs();

// Create file
var output = new FileWriter(output_folder+"/"+bm_interval+".csv");
output.write(header);

// Reading memory
log.log("Reading memory...\n");

//bm_pkts_recv
pkts_per_client = readVariable(server_mote, server_collection, 2).split(";");

var total_pkts_rcvd = 0;
var total_pkts_sent = 0;

// Select mote
for(var k = 0; k < motes.length; k++){
	output.write(""+motes[k].getID()+";");
	stats_value = readVariable(motes[k],motes_statistics,2);
	if(motes[k].getID() == server_id)
		output.write("0;");
	else{
		output.write(pkts_per_client[motes[k].getID()]+";");
		total_pkts_rcvd += parseInt(pkts_per_client[motes[k].getID()]);
		total_pkts_sent += parseInt(stats_value.split(";")[0]);
	}
	output.write(stats_value);
	output.write("\n");
}

//Ouput script total sum (last line), skipping first column
output.write("SUM");
var column = 'B';
var number_of_columns = header.split(";").length - 1;
for(var i=0; i < number_of_columns; i++){
    // Skip first row (should be header)
    output.write(";=SUM("+column+"2:"+column+""+(motes.length+1)+")");
    // Increment column
    column = String.fromCharCode(column.charCodeAt(0) + 1);
}
output.write("\n");

output.write("\n\n\nNetwork path creation time;"+tick2time(startup_time)+"\n");
output.write("Simulation total time;"+tick2time(time - sim_started)+"\n");
//output.write("Defined sim time;"+ms2time(sim_time)+"\n");
//output.write("Defined network settle time;"+ms2time(buffer_clean_delay)+"\n");
output.write("Number of motes;"+motes.length+"\n");
output.write("Network interval;"+bm_interval+"\n");
output.write("Packets/mote;"+bm_packets+"\n");
output.write("Network;"+os_net+"\n");
output.write("OS;"+os_name+"\n");
output.write("Success rate;=100*B"+(motes.length+2)+"/C"+(motes.length+2)+"\n");
output.write("\n\nRoute:\n"+log_route+"\n");

output.close(); /// Save file


//Create a report file with final results
var report = new File(output_folder+"/report.csv");
if(!report.exists()){
	report = new FileWriter(report);
	report.write("Interval (ms);Success (%)\n");
}
else report = new FileWriter(report, true);  //append mode
report.write(bm_interval+";"+(total_pkts_rcvd/total_pkts_sent)*100+"\n");
report.close(); // save the file
log.log("Success rate: "+((total_pkts_rcvd/total_pkts_sent)*100)+"%\n");
//############################################################
//
//  Update parameters for next run
//


// Wait a minimum time between simulations
GENERATE_MSG(delay_between_runs, "sim:sleep");
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg.split(" ")[1]+"\n";
	}
}while(!msg.equals("sim:sleep"));


// Update interval
bm_interval -= bm_interval_dec;
}while(bm_interval >= bm_min_interval);
report.close();

//
//  Simulation ended
//
//############################################################


//############################################################
//
//  End script
//
log.testOK();
