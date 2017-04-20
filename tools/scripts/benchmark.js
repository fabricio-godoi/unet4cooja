// 
//  TODO update this text, put the workflow here
//  - This script starts when the first message arrives through UART, then
// will read the memory that contains the status needed and put it in a 
// log file in csv format;
//  - Note that this script is intended to be used with a little endian MCU,
// and any changes at the code structure (UNET_NodeStat variable) will cause this
// to read the wrong data;
// 
// \brief  Log uNet status in csv file
// \author Fabrício Negrisolo de Godoi
// \date   06-04-2017
// 

//############################################################
// 
//  Simulation parameters
// 
// TIMEOUT must be the first thing to set
TIMEOUT(1800000); // 30 minutes timeout

//Set simulation parameters
var sim_time = 10000; // TODO change it to 10 minutes of simulation at least!
//var sim_time = 10*60*1000; // Simulation time h*m*s*ms
var sim_runs = 10;         // Simulation runs, how many time it'll run

//This will store all motes information (simplify the script)
var motes = sim.getMotes();

// This delay is intended to clear the network buffer (hop/hop)
var buffer_clean_delay = 10000; // one minute // TODO tune it with number of motes

// This is used to create the output log
var sim_type = "uNetDebug";//msg;

//############################################################
//import Java Package to JavaScript
importPackage(java.io);

//############################################################
// 
//  Auxiliary functions and variables definitions
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
// Parse an byte and put it as hex string
function byte2hex(byte) {
	var b2s = ["0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"];
    return ""+(b2s[((byte>>4)&0x0f)])+(b2s[byte&0x0f]);
}

//This will get date YYYYMMDDHHMM
var today = new Date().YYYYMMDDHHMM();

//Open log_<sim>.txt for writing.
//BTW: FileWriter seems to be buffered.
var sim_title = sim.getTitle();

//Creater header (this must be sync with the struct)
var header = "#;" +
		"apptxed;" +
		"apprxed;" +
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
		"radiocol" +
		"\n";

// Benchmark output control commands
// uint8_t start:1; //<! Start communication
// uint8_t stop:1;		//<! Stop communication
// uint8_t stats:1;		//<! Enable/Disable statistics
// uint8_t reset:1;		//<! Reset the statistics
// uint8_t shutdown:1;	//<! Disable any kind of network (not implmentend)
// uint8_t get:1;		//<! Get parameter (not implemented)
// uint8_t set:1;		//<! Set parameter (not implemented)
// uint8_t unused:1;
var BMC_START = 1;
var BMC_STOP  = 1<<1;
var BMC_STATS = 1<<2;
var BMC_RESET = 1<<3;

var BMC_COMMAND = 0;
//Benchmark input control commands
//When received this message from every client node, the simulation can start 
var BM_WAIT_CLIENT = "BMCC_START";

//############################################################
// 
//  Simulation start
// 

//Get simulation type (DEFAULT, FWR_ACK, FIN_BUFF)
//YIELD_THEN_WAIT_UNTIL(msg.equals("UDP server started"));
//YIELD(); // Get next message that contains CONTIKI_CONF_TYPE

log.log("Waiting network stabilization ["+(motes.length-1)+"] ...\n");

// Wait for every client mote to get ready
var started_clients = 0;
do{
	YIELD_THEN_WAIT_UNTIL(msg.equals(BM_WAIT_CLIENT));
	log.log((started_clients+1)+":");
}while(++started_clients < (motes.length - 1));
log.log("\n");

var startup_time = time; // get simulation time

// By simulation definition, mote 0 (zero) must be the coordinator
// Check for the server to start it first
BMC_COMMAND = (0xFF & BMC_START|BMC_STATS);
for(var k = 0; k < motes.length; k++){
	if(motes[k].getID()==0){
		motes[k].getInterfaces().get("Serial").writeByte(BMC_COMMAND);
		break;
	}
}
GENERATE_MSG(100, "sim:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));


// Start all clients
for(var k = 0; k < motes.length; k++){
	if(motes[k].getID()!=0)
		motes[k].getInterfaces().get("Serial").writeByte(BMC_COMMAND);
}
GENERATE_MSG(100, "sim:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));

log.log("Simulation running...\n");

// Wait simulation time
GENERATE_MSG(sim_time, "sim:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));

// Stop all motes messages, but continue the statistics
BMC_COMMAND = (0xFF & BMC_STOP|BMC_STATS);
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeByte(BMC_COMMAND);
}

// Wait buffer delay time
log.log("Waiting network buffer...\n");
GENERATE_MSG(buffer_clean_delay, "sim:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim:sleep"));

// Stop all motes statistics
BMC_COMMAND = (0xFF & BMC_STOP);
for(var k = 0; k < motes.length; k++){
	motes[k].getInterfaces().get("Serial").writeByte(BMC_COMMAND);
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
var output = new FileWriter("/home/user/logs/log_"+sim_title+"_"+sim_type+"_"+today+".csv");
output.write(header);

// Select mote
log.log("Reading memory:\n");
var _bytes = 2; //16 bits variable
for(var k = 0; k < motes.length; k++){
	log.log(motes[k].getID()+":");
	output.write(""+motes[k].getID());
	
	// Find variable by name
	_var = motes[k].getMemory().getSymbolMap().get("UNET_NodeStat");
	//log.log(_var.name+" = "+_var.addr+"["+_var.size+"]\n");
	
	// Get value and transform it to number
	var value = motes[k].getMemory().getMemorySegment(_var.addr, _var.size);
	var i = 0, j = 0;
	var hex;
	var number;
	for(i=0;i<value.length;i+=_bytes){
		// Since values read from memory is signed (-128 to 127)
		// transform it into hex string and parse to be from 0 to 255
		hex = "";
		for(var m = _bytes-1; m >= 0; m--)
			hex += byte2hex(value[i+m]);
		output.write(";"+parseInt(hex,16));
	}
	output.write("\n");
}
log.log("\n");

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

output.write("\n\n\nNetwork stabilization time;"+startup_time+"\n");
output.write("Simlation time;"+time+"\n");
output.write("Number of motes;"+motes.length+"\n");

output.close(); /// Save file

//############################################################
//
//  End script
//
log.testOK();
