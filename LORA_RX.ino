/**
 * Beispiel: Reiner LoRa-Empfänger.
 * Dieser Code initialisiert das LoRa-Modul und wartet kontinuierlich auf eingehende Pakete.
 * Empfangen werden die Daten sowie RSSI und SNR, die direkt über das integrierte Display und den Seriellen Monitor ausgegeben werden.
 */
#include <heltec_unofficial.h>

// ----- Konfiguration -----
#define FREQUENCY           866.3         // Frequenz in MHz (Europa)
#define BANDWIDTH           250.0         // LoRa-Bandbreite in kHz
#define SPREADING_FACTOR    9             // LoRa-Spreading Factor (5 bis 12)
#define TRANSMIT_POWER      0            // Sendeleistung (wird hier zwar gesetzt, aber im Empfang nicht genutzt)

// ----- Globale Variablen -----
String rxData;                           // Empfangene Daten
volatile bool rxFlag = false;            // Flag, wenn ein Paket empfangen wurde

float temp = 0.0;
float pre = 0.0;
int PacketId = 0;

// Callback-Funktion, die per Interrupt aufgerufen wird, wenn ein Paket ankommt.
// Da in ISR-Funktionen möglichst wenig Arbeit erfolgen sollte, wird nur ein Flag gesetzt.
void rx() 
{
  rxFlag = true;
}

void parseMessage(const String &message) 
{
  // Umwandeln der Nachricht in ein C-String
  const char* msg = message.c_str();
  // Versuche, die Werte zu parsen:
 int parsed = sscanf(msg, "T:%fC, P:%fhPa, ID:%d", &temp, &pre, &PacketId);  if (parsed == 3) 
  {
    //Hier eine Ausgabe der Daten, kann "muss" aber nicht, da Daten im Loop auch ausgegeben werden
    both.printf("Parsed: Temp: %.2f C, Pre: %.2f%%, ID: %d\n", temp, pre, PacketId);
  } 
  else 
  {
    both.println("Parsing error!");
  }
}

void setup() 
{
  heltec_setup();
  
  both.println("Radio init");
  RADIOLIB_OR_HALT(radio.begin());
  
  // Setze den Interrupt-Callback für empfangene Pakete
  radio.setDio1Action(rx);
  
  // Konfiguriere die Radio-Parameter
  both.printf("Frequenz: %.2f MHz\n", FREQUENCY);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  both.printf("Bandbreite: %.1f kHz\n", BANDWIDTH);
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  //Keine Ausgabe der eingestellten Leistung, da nur Empfänger Code
  // both.printf("TX-Leistung: %i dBm\n", TRANSMIT_POWER);
  // RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  
  // Starte den kontinuierlichen Empfang
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
}

void loop() 
{
  heltec_loop();
  
  // Wird ein Paket empfangen, so wird das Flag im Callback gesetzt
  if (rxFlag) 
  {
    rxFlag = false;
    radio.readData(rxData);

    parseMessage(rxData);
    Serial.println("------------------");
    Serial.print("DataRx Temp:");
    Serial.print(temp);
    Serial.print(" Pre: ");
    Serial.print(pre);
    Serial.print(" PacketID: ");
    Serial.println(PacketId);
    Serial.println("--------------------");

    if (_radiolib_status == RADIOLIB_ERR_NONE) 
    {
      both.printf("RX: [%s]\n", rxData.c_str());
      both.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
      both.printf("  SNR: %.2f dB\n", radio.getSNR());
     // both.println();  // Gibt eine leere Zeile aus
    } 
    else 
    {
      both.printf("Empfangsfehler (%i)\n", _radiolib_status);
    }
    // Nach der Verarbeitung wieder in den Empfangsmodus wechseln
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }
}
