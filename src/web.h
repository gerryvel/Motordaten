
#include "helper.h"
#include "configuration.h"
#include "boardinfo.h"



//Variables for website
String sCL_Status = sWifiStatus(WiFi.status());
String replaceVariable(const String& var)
{
	if (var == "sKraengung")return String(fDrehzahl,1);
	if (var == "sGaugeKraengung")return String(fGaugeDrehzahl, 1);
	if (var == "sSStellung")return String(fSStellung,1);
	if (var == "sAbsTief")return String(fAbsTief,1);
	if (var == "sSTBB")return sSTBB;
	if (var == "sBoardInfo")return sBoardInfo;
	if (var == "sFS_Space")return String(LittleFS.usedBytes());
	if (var == "sAP_IP")return String(tAP_Config.wAP_IP);
  	if (var == "sAP_Clients")return String(WiFi.softAPgetStationNum());
  	if (var == "sCL_Addr")return WiFi.localIP().toString();
  	if (var == "sCL_Status")return String(sCL_Status);
  	if (var == "sI2C_Status")return String(sI2C_Status);
	if (var == "CONFIGPLACEHOLDER")return processor(var);
  	return "NoVariable";
}

String processor(const String& var)
{
	if (var == "CONFIGPLACEHOLDER")
	{
		String buttons = "";
		buttons += "<form onSubmit = \"event.preventDefault(); formToJson(this);\">";
		buttons += "<p class=\"CInput\"><label>SSID </label><input type = \"text\" name = \"SSID\" value=\"";
		buttons += tAP_Config.wAP_SSID;
		buttons += "\"/></p>";
		buttons += "<p class=\"CInput\"><label>IP </label><input type = \"text\" name = \"IP\" value=\"";
		buttons += tAP_Config.wAP_IP;
		buttons += "\"/></p>";
		buttons += "<p class=\"CInput\"><label>Password </label><input type = \"text\" name = \"Password\" value=\"";
		buttons += tAP_Config.wAP_Password;
		buttons += "\"/></p>";
		buttons += "<p class=\"CInput\"><label>Kieloffset </label><input type = \"text\" name = \"Kiel_Offset\" value=\"";
		buttons += tAP_Config.wTemp_Offset;
		buttons += "\"/> cm</p>";
		buttons += "<p><input type=\"submit\" value=\"Speichern\"></p>";
		buttons += "</form>";
		return buttons;
	}
	return String();
}

// Info Board for HTML-Output
String sBoardInfo;
BoardInfo boardInfo;

