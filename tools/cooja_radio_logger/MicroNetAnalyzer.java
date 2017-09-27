package org.contikios.cooja.plugins.analyzers;

import java.io.IOException;
import java.io.File;
import java.util.Arrays;

import org.apache.log4j.Logger;
import org.contikios.cooja.util.StringUtils;

public class MicroNetAnalyzer extends PacketAnalyzer {

	private static final Logger logger = Logger.getLogger(PacketAnalyzer.class);

	// Addressing modes
	public static final int NO_ADDRESS = 0;
	public static final int RSV_ADDRESS = 1;
	public static final int SHORT_ADDRESS = 2;
	public static final int LONG_ADDRESS = 3;

	// 802.15.4 Frame types
	public static final int BEACONFRAME = 0x00;
	public static final int DATAFRAME = 0x01;
	public static final int ACKFRAME = 0x02;
	public static final int CMDFRAME = 0x03;

	// uNet LLC
	public static final int UNET_LLC = 0x01;

	// uNet packet types
	public static final int BROADCAST = 0x00;
	public static final int UNICAST_ACK_DOWN = 0x01;
	public static final int UNICAST_ACK_UP = 0x02;
	public static final int UNICAST_DOWN = 0x05;
	public static final int UNICAST_UP = 0x06;
	public static final int MULTICAST_ACK_UP = 0x0A;
	public static final int MULTICAST_UP = 0x0E;

	// uNet headers
	public static final int HEADER_NONE = 0x3B;
	public static final int HEADER_TCP = 0x06;
	public static final int HEADER_UDP = 0x11;
	public static final int HEADER_UNET_CTRL = 0xFD;
	public static final int HEADER_UNET_APP = 0xFE;

	// private static final byte[] BROADCAST_ADDR = {(byte)0xff, (byte)0xff};
	private static final String[] typeS = { "-", "D", "A", "C" };
	private static final String[] typeVerbose = { "BEACON", "DATA", "ACK",
			"CMD" };
	private static final String[] addrModeNames = { "None", "Reserved",
			"Short", "Long" };

	public MicroNetAnalyzer() {
	}

	@Override
	public boolean matchPacket(Packet packet) {
		// System.out.println("UNET MATCH PACKET");
		return true;
		// return packet.level == MAC_LEVEL;
	}

	/* this protocol always have network level packets as payload */
	public int nextLevel(byte[] packet, int level) {
		return APPLICATION_LEVEL;
	}

	/*
	 * create a 802.15.4 packet of the bytes and "dispatch" to the next handler
	 */

	@Override
	public int analyzePacket(Packet packet, StringBuilder brief,
			StringBuilder verbose) {

		int pos = packet.pos;
		// FCF field
		int fcfType = packet.data[pos + 0] & 0x07;
		boolean fcfSecurity = ((packet.data[pos + 0] >> 3) & 0x01) != 0;
		boolean fcfPending = ((packet.data[pos + 0] >> 4) & 0x01) != 0;
		boolean fcfAckRequested = ((packet.data[pos + 0] >> 5) & 0x01) != 0;
		boolean fcfIntraPAN = ((packet.data[pos + 0] >> 6) & 0x01) != 0;
		int fcfDestAddrMode = (packet.data[pos + 1] >> 2) & 0x03;
		int fcfFrameVersion = (packet.data[pos + 1] >> 4) & 0x03;
		int fcfSrcAddrMode = (packet.data[pos + 1] >> 6) & 0x03;
		// Sequence number
		int seqNumber = packet.data[pos + 2] & 0xff;
		// Addressing Fields
		int destPanID = 0;
		int srcPanID = 0;
		byte[] sourceAddress = null;
		byte[] destAddress = null;
		int srcAddr = 0;
		int dstAddr = 0;

		pos += 3;

		if (fcfDestAddrMode > 0) {
			destPanID = (packet.data[pos] & 0xff)
					+ ((packet.data[pos + 1] & 0xff) << 8);
			pos += 2;
			if (fcfDestAddrMode == SHORT_ADDRESS) {
				destAddress = new byte[2];
				destAddress[1] = packet.data[pos];
				destAddress[0] = packet.data[pos + 1];
				dstAddr = ((destAddress[0] & 0xFF) << 8)
						+ (destAddress[1] & 0xFF);
				pos += 2;
			} else if (fcfDestAddrMode == LONG_ADDRESS) {
				destAddress = new byte[8];
				for (int i = 0; i < 8; i++) {
					destAddress[i] = packet.data[pos + 7 - i];
				}
				pos += 8;
			}
		}

		if (fcfSrcAddrMode > 0) {
			if (fcfIntraPAN) {
				srcPanID = destPanID;
			} else {
				srcPanID = (packet.data[pos] & 0xff)
						+ ((packet.data[pos + 1] & 0xff) << 8);
				pos += 2;
			}
			if (fcfSrcAddrMode == SHORT_ADDRESS) {
				sourceAddress = new byte[2];
				sourceAddress[1] = packet.data[pos];
				sourceAddress[0] = packet.data[pos + 1];
				srcAddr = ((sourceAddress[0] & 0xFF) << 8)
						+ (sourceAddress[1] & 0xFF);
				pos += 2;
			} else if (fcfSrcAddrMode == LONG_ADDRESS) {
				sourceAddress = new byte[8];
				for (int i = 0; i < 8; i++) {
					sourceAddress[i] = packet.data[pos + 7 - i];
				}
				pos += 8;
			}
		}

		verbose.append("<html><b>IEEE 802.15.4 ")
				.append(fcfType < typeVerbose.length ? typeVerbose[fcfType]
						: "?").append("</b> #").append(seqNumber);

		if (fcfType != ACKFRAME) {

			verbose.append("<br>[PID: 0x")
					.append(StringUtils.toHex((byte) (srcPanID >> 8)))
					.append(StringUtils.toHex((byte) (srcPanID & 0xff)));
			if (!fcfIntraPAN) {
				verbose.append(" to 0x")
						.append(StringUtils.toHex((byte) (destPanID >> 8)))
						.append(StringUtils.toHex((byte) (destPanID & 0xff)));
			}
			verbose.append("] ");

			verbose.append("[DST: " + (srcAddr == 65535 ? "BCAST" : dstAddr)
					+ "] ");
			// printAddress(verbose, fcfDestAddrMode, destAddress);
			verbose.append("[SRC: " + (srcAddr == 65535 ? "BCAST" : srcAddr)
					+ "] ");
			// printAddress(verbose, fcfSrcAddrMode, sourceAddress);
		}

		verbose.append("<br>Sec = ").append(fcfSecurity).append(", Pend = ")
				.append(fcfPending).append(", ACK = ").append(fcfAckRequested)
				.append(", iPAN = ").append(fcfIntraPAN)
				.append(", DestAddr = ").append(addrModeNames[fcfDestAddrMode])
				.append(", Vers. = ").append(fcfFrameVersion)
				.append(", SrcAddr = ").append(addrModeNames[fcfSrcAddrMode]);

		// Check uNet LLC
		if (packet.data[pos] == UNET_LLC && fcfType != ACKFRAME) {
			pos++;

			// uNet Network
			int unet_pkt_type = packet.data[pos++] & 0xFF;
			int unet_pkt_len = packet.data[pos++] & 0xFF;
			int unet_hop_limit = packet.data[pos++] & 0xFF;
			int unet_next_hdr = packet.data[pos++] & 0xFF;

			verbose.append("<br><br><b>uNet Network</b><br>");
			verbose.append("[TYPE: " + unet_type_string(unet_pkt_type) + "] ");
			verbose.append("[LENG: " + unet_pkt_len + "] ");
			verbose.append("[HOP LIMIT: " + unet_hop_limit + "] ");
			verbose.append("[NEXT HEADER: "
					+ unet_nhheader_string(unet_next_hdr) + "]");

			// uNet Transport
			verbose.append("<br><br><b>uNet Transport</b>");
			byte[] unet_srce = new byte[8];
			byte[] unet_dest = new byte[8];
			for (int i = 0; i < 8; i++) {
				unet_srce[i] = packet.data[pos + i];
			}
			pos += 8;
			for (int i = 0; i < 8; i++) {
				unet_dest[i] = packet.data[pos + i];
			}
			pos += 8;

			verbose.append("<br>Source: ");
			printAddress(verbose, LONG_ADDRESS, unet_srce);
			verbose.append(" [Node "
					+ (((unet_srce[6] & 0xFF) << 8) + (unet_srce[7] & 0xFF))
					+ "]");
			verbose.append("</br><br>Destiny: ");
			printAddress(verbose, LONG_ADDRESS, unet_dest);
			verbose.append(" [Node "
					+ (((unet_dest[6] & 0xFF) << 8) + (unet_dest[7] & 0xFF))
					+ "]");

			// uNet Application
			if (unet_next_hdr == HEADER_UNET_APP
					&& (unet_pkt_type == UNICAST_DOWN || unet_pkt_type == UNICAST_UP)) {
				int unet_src_port = packet.data[pos++] & 0xFF;
				int unet_dst_port = packet.data[pos++] & 0xFF;
				int unet_app_len = packet.data[pos++] & 0xFF;
				verbose.append("<br><br><b>uNet Application</b>");
				verbose.append("<br>[SPort " + unet_src_port + "][DPort "
						+ unet_dst_port + "]");
				verbose.append("[APP LEN " + unet_app_len + "]");
			}
		}

		// Keep orinal packet in the brief
		for (int i = 0; i < packet.size(); i++) {
			brief.append(StringUtils.toHex(packet.data[i]));
			if ((i + 1) % 4 == 0)
				brief.append(" ");
		}

		/* update packet */
		packet.pos = pos;

		/* remove CRC from the packet */
		packet.consumeBytesEnd(2);

		// for(int i = 0; i < packet.size(); i++){
		// verbose.append(StringUtils.toHex(packet.data[pos+i]));
		// if((i+1)%4 == 0)verbose.append(" ");
		// if((i+1)%20 == 0)verbose.append(" "+);
		//
		// }

		// if (fcfType != ACKFRAME) {
		/* put the raw data above */
		// byte[] rawData = Arrays.copyOfRange(packet.data, pos,
		// pos + packet.size());
		// verbose.append("<br><br><b>Payload</b><br>");
		// verbose.append("<pre>" + StringUtils.hexDump(rawData) + "</pre>");
		// }

		packet.level = APPLICATION_LEVEL;
		packet.llsender = sourceAddress;
		packet.llreceiver = destAddress;
		return ANALYSIS_OK_FINAL;
	}

	private void printAddress(StringBuilder sb, int type, byte[] addr) {
		if (type == SHORT_ADDRESS) {
			sb.append("0x").append(StringUtils.toHex(addr));
		} else if (type == LONG_ADDRESS) {
			sb.append(StringUtils.toHex(addr[0])).append(':')
					.append(StringUtils.toHex(addr[1])).append(':')
					.append(StringUtils.toHex(addr[2])).append(':')
					.append(StringUtils.toHex(addr[3])).append(':')
					.append(StringUtils.toHex(addr[4])).append(':')
					.append(StringUtils.toHex(addr[5])).append(':')
					.append(StringUtils.toHex(addr[6])).append(':')
					.append(StringUtils.toHex(addr[7]));

		}
	}

	private String unet_type_string(int i) {
		switch (i) {
		case BROADCAST:
			return "BROADCAST";
		case UNICAST_ACK_DOWN:
			return "UNI ACK DOWN";
		case UNICAST_ACK_UP:
			return "UNI ACK UP";
		case UNICAST_DOWN:
			return "UNI DOWN";
		case UNICAST_UP:
			return "UNI UP";
		case MULTICAST_ACK_UP:
			return "MULT ACK UP";
		case MULTICAST_UP:
			return "MULT UP";
		default:
			return "UNKOWN";
		}
	}

	private String unet_nhheader_string(int i) {
		switch (i) {
		case HEADER_NONE:
			return "NONE";
		case HEADER_TCP:
			return "TCP";
		case HEADER_UDP:
			return "UDP";
		case HEADER_UNET_CTRL:
			return "UNET_CTRL";
		case HEADER_UNET_APP:
			return "UNET_APP";
		default:
			return "UNKOWN";
		}
	}
}
