// 
// \brief  Read memory and check progress
// \author Fabrício Negrisolo de Godoi
// \date   2017-10-18
// Obs.:
//  Serial at: cooja.interfaces.SerialPort.java
//  Memory at: cooja.mote.memory.MemoryInterface.java
//

//############################################################
// 
//  Simulation parameters
// 
// TIMEOUT must be the first thing to set
TIMEOUT(86400000); // 24 hours timeout

// This will store all motes information (simplify the script)
var motes = sim.getMotes();

//Number of packets by each mote
var bm_packets = 100;	// value should not exceed 127

// Statistcs variables
var motes_statistics = "UNET_NodeStat";
var server_collection = "bm_pkts_recv";
var server_id = 0;
var server_mote;

for(var i=0; i < motes.length; i++){
	if(motes[i].getID() == server_id){
		server_mote = motes[i];
	}
}


//############################################################
//
//  Auxiliary functions definitions
//

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

//var progress = new loading();
//progress.start();
//progress.load((started_clients)/(motes.length-1));
//progress.end();

//############################################################
//
//  Extract data from motes
//

// Wait system wake up
GENERATE_MSG(5000, "sim_progress:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim_progress:sleep"));

// Reading memory
log.log("Info:\n");
log.log("     Number of motes: "+motes.length+"\n");
log.log("     Packets/mote: "+bm_packets+"\n");
log.log("     Total of packets: "+(bm_packets*(motes.length-1))+"\n\n");

do{

//bm_pkts_recv
pkts_per_client = readVariable(server_mote, server_collection, 2).split(";");

var total_pkts_rcvd = 0;
var total_pkts_sent = 0;
var pkts_per_client_sent = [];
for(var i=0; i < motes.length; i++) pkts_per_client_sent[i] = 0;
// Select mote
for(var k = 0; k < motes.length; k++){
	//output.write(""+motes[k].getID()+";");
	stats_value = readVariable(motes[k],motes_statistics,2);
	if(motes[k].getID() == server_id){}
	else{
		total_pkts_rcvd += parseInt(pkts_per_client[motes[k].getID()]);
		total_pkts_sent += parseInt(stats_value.split(";")[0]);
	}
	//pkts_per_client_sent[k] = parseInt(pkts_per_client[motes[k].getID()]);
}

log.log("Progress: "+(100*total_pkts_sent/(bm_packets*(motes.length-1))).toFixed(2)+"% ("+total_pkts_sent+"/"+(bm_packets*(motes.length-1))+")\n");
log.log("Success rate: "+((total_pkts_rcvd/total_pkts_sent)*100).toFixed(2)+"%\n\n");


var p=1;
log.log("PktRcv |  000  |  001  |  002  |  003  |  004  |  005  |  006  |  007  |\n");
log.log("  Dec.  |---------------------------------------------------------------------------------------|");
for(var i=0; i<motes.length; i++){
    if((i)%8==0){
	log.log("\n  "+("000"+i).substr(-3)+"   |");
    }
    log.log("  "+("000"+pkts_per_client[i]).substr(-3)+"  |");
}
log.log("\n           |---------------------------------------------------------------------------------------|\n\n");

// Wait 15s to refresh the data
GENERATE_MSG(5000, "sim_progress:sleep");
YIELD_THEN_WAIT_UNTIL(msg.equals("sim_progress:sleep"));

}while(1);

