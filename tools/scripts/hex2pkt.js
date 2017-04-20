/*
 * @brief Script to read hex from radio stdio and transform it into readable packet
 * @author Fabricio Negrisolo de Godoi
 * @date 26-03-2017
 */

//import Java Package to JavaScript
importPackage(java.io);

TIMEOUT(99999999); // Set to infinity (disable timeout) (~1.15days)

function time2string(_time){
	var string = "";
	_time=Math.floor(_time/1000);
	var min  = Math.floor(_time/3600000);
	var sec  = Math.floor((_time%3600000)/1000);
	var msec = _time - min*3600000 - sec*1000;
	string=min+":"+sec+"."+msec;
	return string;	
}

function unet_pkt_type(type){
	if(type.equals("00")) return "BROADCAST";
	if(type.equals("01")) return "UNICAST ACK DOWN";
	if(type.equals("02")) return "UNICAST ACK UP";
	if(type.equals("05")) return "UNICAST DOWN";
	if(type.equals("00")) return "UNICAST UP";
	if(type.equals("0A")) return "MULTICAST ACK UP";
	if(type.equals("0E")) return "MULTICAST UP";
	return "";
}
function unet_header(header){
	if(header.equals("3B")) return "NONE";
	if(header.equals("06")) return "TCP";
	if(header.equals("11")) return "UDP";
	if(header.equals("FD")) return "UNET CONTROL";
	if(header.equals("FE")) return "UNET APP";
	if(header.equals("FF")) return "RESERVED";
	return "";
}

//// Function to transform any hexadecimal value to byte value (Ex.: FF to 255)
//function hex2byte(hex){
//	var byte=0;
//	var _i
//	for(_i=0;_i<hex.length;_i++){
//		c=hex.charCodeAt(_i);
//		if(c >= "0".charCodeAt(0) && c <= "9".charCodeAt(0)){
//			c=c-"0".charCodeAt(0);
//		} else if(c >= "a".charCodeAt(0) && c <= "f".charCodeAt(0)){
//			c=c-"a".charCodeAt(0);
//		} else if(c >= "A".charCodeAt(0) && c <= "F".charCodeAt(0)){
//			c=c-"A".charCodeAt(0);
//		} else {
//			c=0;
//		}
//		log.log("c = "+c+"\n");
//		byte=(byte<<(4*_i))|c;
//	}
//	log.log("return = "+byte+"\n");
//	return byte;
//}



log.log("==========================================================\n");
var hex;
var i,j;

while(true){

// Get simulation type (DEFAULT, FWR_ACK, FIN_BUFF)
YIELD_THEN_WAIT_UNTIL(msg.startsWith("cc2520: pkt: ")||msg.startsWith("61 ")||msg.startsWith("41 "));
hex = msg.replace("cc2520: pkt: ", "").split(" ");

log.log(time2string(time)+" | ID:"+mote.getID()+" | Length: "+hex.length+" Bytes\n");
log.log((msg.replace("cc2520: pkt: ",""))+"\n");

i=0;
while(1){ // same as goto
log.log("MAC Frame Ctrl: "+hex[i++]+hex[i++]+"\n"); if(i==hex.length) break;
log.log("MAC Seq Num: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("MAC PAN ID: "+hex[i++]+hex[i++]+"\n"); if(i==hex.length) break;
log.log("MAC Dest: "+hex[i++]+hex[i++]+"\n"); if(i==hex.length) break;
log.log("MAC Source: "+hex[i++]+hex[i++]+"\n"); if(i==hex.length) break;
log.log("LLC: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("uN[N] Pkt Type: "+hex[i]+" : "+unet_pkt_type(hex[i++])+"\n"); if(i==hex.length) break;
log.log("uN[N] Payload Len: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("uN[N] Hop Lim: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("uN[N] Next Header: "+hex[i]+" : "+unet_header(hex[i++])+"\n"); if(i==hex.length) break;
log.log("uN[N] Source: "); for(j=0;j<8;j++)log.log(hex[i++]); log.log("\n"); if(i==hex.length) break;
log.log("uN[N] Dest: "); for(j=0;j<8;j++)log.log(hex[i++]); log.log("\n"); if(i==hex.length) break;
log.log("uN[T] S Port: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("uN[T] D Port: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("uN[T] App Len: "+hex[i++]+"\n"); if(i==hex.length) break;
log.log("App:"); for(j=i;j<hex.length;j++)log.log(" "+hex[j]); log.log("\n");
break;
}

log.log("==========================================================\n");
}

log.testOK(); // End script with success