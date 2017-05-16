// 
// \brief  Log mote statistics in csv file
// \author Fabrício Negrisolo de Godoi
// \date   06-04-2017
//
// \details
//   This script is performed in 4 steps:
// 1st - Definition of used variables and function, in which:
//       - motes_statistcs is the variable that store all motes statistics,
//         "header" variable must be the same type of this structure;
//       - server_collection is a server variable informing the packets
//         received by each client;
//       - "header" must contains the exact format of the statistics
//         variable in the mote write in .csv;
//       - server information must got from server mote from UART in 
//         this exact format: "server: os_name longest_path"
// 2nd - Simulation initialization (UART):
//       - Get server information and inform each mote the network size
// 3rd - Simulation running (UART):
//       - Wait the network stabilization time ...
//           ... Each client has a path to server + delay time (10s)
//       - Start all motes (clean statistics and start the processes) ...
//           ... wait run time defined by sim_time;
//       - After sim_time, stop all motes (statistcs and processes)
// 4th - Results collections (RAM):
//       - Read the motes memory collecting the statistics specified by
//         header directly from the memory;
//
// Obs.:
//  Serial at: cooja.interfaces.SerialPort.java
//  Memory at: cooja.mote.memory.MemoryInterface.java
//

//############################################################
// 
//  Simulation parameters
// 
// TIMEOUT must be the first thing to set
TIMEOUT(3600000); // 60 minutes timeout

//Set simulation parameters
var sim_time = 10*60*1000; // Simulation time h*m*s*ms

//This will store all motes information (simplify the script)
var motes = sim.getMotes();

// This delay is intended to clear the network buffer (hop/hop)
var buffer_clean_delay = 1000; // wait one second to clear the buffer

// This is used to create the output log
var os_name = "none";
var os_net  = "none";
var net_pkts= "null"
var server_id = 0; // default
var server_mote = null;
var sim_title = sim.getTitle();

// Statistcs variables
var motes_statistics = "UNET_NodeStat";
var server_collection = "bm_pkts_recv";

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
	var minutes = Math.floor(t/(1000*1000*60))%60;
	var seconds = Math.floor(t/(1000*1000))%60;
	var milisec = Math.floor(t/1000)%1000;
	return ""+("0"+minutes).slice(-2)+":"+("0"+seconds).slice(-2)+"."+("00"+milisec).slice(-3);
}
//Parse time into mm:ss.mss
function ms2time(t){
	var minutes = Math.floor(t/(1000*60))%60;
	var seconds = Math.floor(t/(1000))%60;
	var milisec = t%1000;
	return ""+("0"+minutes).slice(-2)+":"+("0"+seconds).slice(-2)+"."+("00"+milisec).slice(-3);
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
var log_route = "timestamp;id;route\n";

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
		"radiotx;" +
		"radiorx;" +
		"radiocol;" +
		"radionack" +
		"\n";

// Benchmark output control commands
// uint8_t start:1;     //<! Start communication
// uint8_t stop:1;      //<! Stop communication
// uint8_t stats:1;     //<! Enable/Disable statistics
// uint8_t reset:1;     //<! Reset the statistics
// uint8_t shutdown:1;  //<! Disable any kind of network (not implmentend)
// uint8_t get:1;       //<! Get parameter (not implemented)
// uint8_t set:1;       //<! Set parameter (not implemented)
// uint8_t unused:1;
var BMC_START    = 1;
var BMC_STOP     = 1<<1;
var BMC_STATS    = 1<<2;
var BMC_RESET    = 1<<3;
var BMC_SHUTDOWN = 1<<4;
var BMC_GET      = 1<<5;
var BMC_SET      = 1<<6;

var BMC_COMMAND = [];
var BCM_DATA    = (motes.length-1) & 0xFF;
var BCM_END     = 0x0a; // Ends with '\n'

//############################################################
//
//   Simulation initialization
//

// Get some simulation information
YIELD_THEN_WAIT_UNTIL(msg.startsWith("server:"));
server_id = id;
var server_mote = mote;
var string = msg.replace("server: ","").split(" ");
os_name = string[0]; os_net = string[1]; longest_path = string[2];
log.log("==========================================\n");
log.log("Simulation Parameters\n");
log.log("Server ID: "+server_id+"\n");
log.log("Server OS: "+os_name+"\n");
log.log("OS Network: "+os_net+"\n");
log.log("Longest path (Hops): "+longest_path+"\n");
log.log("Minimum transmission period Tm = H * 10ms: "+(longest_path*10)+"ms\n");
log.log("Delay between packets: "+(longest_path*10*(motes.length-1))+"ms\n");
log.log("Total packets expected per client: "+Math.round(sim_time/(longest_path*10*(motes.length-1)))+"\n");
log.log("==========================================\n");

//Benchmark input control commands
//When received this message from every client node, the simulation can start 
var BM_WAIT_CLIENT = "BMCC_START";

/// Notify all clients with the network size
//Wait system wake up
GENERATE_MSG(1000, "sim:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));
// First, send the command
BMC_COMMAND = [(0xFF & BMC_SET), ((motes.length-1) & 0xFF), BCM_END];
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}

//############################################################
// 
//  Simulation start
// 

// Network stabilization
log.log("Waiting network stabilization ["+(motes.length-1)+"] ...\n");

//Wait for every client mote to get ready
var load = new loading();
load.start();
var started_clients = 0;
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals(BM_WAIT_CLIENT) || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg+"\n";
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
GENERATE_MSG(10000, "sim:sleep");
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg+"\n";
	}
}while(!msg.equals("sim:sleep"));

log.log("Stabilization time: "+tick2time(time)+"\n");
log.log("Simulation expected to finish at: "+tick2time(time+sim_time*1000)+"\n");

//Start all motes, server first
BMC_COMMAND = [(0xFF & BMC_START|BMC_STATS), BCM_END];
server_mote.getInterfaces().get("Serial").writeArray(BMC_COMMAND);
for(var k = 0; k < motes.length; k++){
	if(motes[k].getID()!=server_id)
		motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}

log.log("Simulation running...\n");

// Wait simulation time
GENERATE_MSG(sim_time, "sim:sleep");
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg+"\n";
	}
}while(!msg.equals("sim:sleep"));

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
//YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep") || msg.startsWith("#L "));
	if(msg.startsWith("#L ")){
		msg = msg.split(";")[0];
		if(msg.split(" ")[2].equals("1"))
			log_route += tick2time(time)+";"+id+";"+msg+"\n";
	}
}while(!msg.equals("sim:sleep"));

// Stop all motes statistics
BMC_COMMAND = [(0xFF & BMC_STOP), BCM_END];
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeArray(BMC_COMMAND);
}

//
//  Simulation ended
//
//############################################################


//############################################################
//
//  Extract data from motes
//

log.log("Retrieving data from motes ["+motes.length+"] ...\n");

// Create file
var output = new FileWriter("/home/user/logs/log_"+sim_title+"_"+os_name+"_"+os_net+"_"+today+".csv");
output.write(header);

// Reading memory
log.log("Reading memory:\n");

//bm_pkts_recv
pkts_per_client = readVariable(server_mote, server_collection, 2).split(";");

// Select mote
var load = new loading();
load.start();
for(var k = 0; k < motes.length; k++){
	output.write(""+motes[k].getID()+";");
	if(motes[k].getID() == server_id)
		output.write("0;");
	else
		output.write(pkts_per_client[motes[k].getID()]+";");
	output.write(readVariable(motes[k],motes_statistics,2));
	output.write("\n");
	load.load((k+1)/motes.length);
}
load.end();

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
output.write("Simulation total time;"+tick2time(time)+"\n");
output.write("Defined sim time;"+ms2time(sim_time)+"\n");
output.write("Defined network settle time;"+ms2time(buffer_clean_delay)+"\n");
output.write("Number of motes;"+motes.length+"\n");
output.write("Defined server pkt/s;"+net_pkts+"\n");
output.write("Network;"+os_net+"\n");
output.write("Success rate;=100*B"+(motes.length+2)+"/C"+(motes.length+2)+"\n");
output.write("\n\nRoute:\n"+log_route+"\n");

output.close(); /// Save file


//############################################################
//
//  End script
//
log.testOK();
