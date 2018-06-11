#include "stdafx.h"
#include "SensorData.h"
#include <time.h>

using namespace std;

SensorData::SensorData(int c, int rssi, int sec, int usec, string s, string seq_ctrl, string ssid) :
	channel(c), RSSI(rssi), seconds(sec), useconds(usec), source(s), sequence_ctrl(seq_ctrl), SSID(ssid) {

}

SensorData::~SensorData() {

}

void SensorData::printData() {
	time_t rawtime = seconds;
	const int buffer_size = 256;
	char buffer[buffer_size];

	struct tm timeinfo;

	errno_t result = localtime_s(&timeinfo, &rawtime);
	asctime_s(buffer, buffer_size, &timeinfo);

	cout << "PACKET TYPE=PROBE CHAN=" << channel << " RSSI=" << RSSI
		<< " ADDR=" << source << " SEQ=" << sequence_ctrl
		<< " SSID=" << SSID  << " Time= " << buffer << endl;

	return;
}
