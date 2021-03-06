/* Universeller DCC - Zubehördecoder
 * 3 Ausgänge / Zubehöradresse
 * Einstellbare Funktionalität:
 *  - Servo mit Umschaltrelais zur Weichenpolarisierung
 *  - Doppelspulenantriebe
 *  - statische/blinkende Ausgänge 
 *  - Lichtsignale 
 *  - Einstellen der Servoendlagen per Drehencoder. 
 *    Der Drehencoder bezieht sich immer auf die zuletzt gestellte Weichenposition .
 *  - Die Betriebsmodi und Startverhalten wird über die Analogeingänge A4/A5 (parametrierbar) eingestellt. Dazu 
 *    müssen dort Pullups eingebaut werden. Jenachdem wieweit die Spannung  heruntergezogen wird werden
 *    die Modi eingestellt:
 *     A5:   5V (offen) normaler Betriebsmodus, kein PoM
 *           3,3V (Spannungsteiler 1:2) PoM immer aktiv, Adresse immer aus defaults
 *           1,6V (Spannungsteiler 2:1) IniMode: CV's werden immer auf init-Werte aus .h-Datei gesetzt
 *           0V Programmiermodus / PoM ( 1. Empfamgenes Telegramm bestimmt Adresse )
 *     A4:   wird A4 auf 0 gezogen , wird der aktuell vom Drehencoder beeinflusste Servo in die  
 *           Mittellage gebracht. Sobald der Encoder wieder bewegt wird, bewegt sich das Servo wieder
 *           zur vorhergehenden Position.
 *           Ist A4 beim Programmstart auf 0, werden alle CV's auf die Defaults zurückgesetzt
 *                  
 * Eigenschaften:
 * Mehrere (aufeinanderfolgende) Zubehöradressen ansteuerbar. Die mögliche Zahl von Adressen hängt 
 * von den verfügbaren Digitalausgängen ab.
 * Es können maximal 16 Servos angesteuert werden
 * 
 * 1. Adresse per Programmierung einstellbar
 * 
 *  Das Verhalten der konfigurierten Funktionen wird über CV-Programmierung festgelegt: 
 *  Bei Servoausgängen die Endlagen und die Geschwindigkeit
 *  bei Doppelspulenantrieben die Einschaltzeit der Spulen.
 *  bei blinkenden Ausgängen das Blinkverhalten ( in V3.0 noch nicht realisiert )
 *  
 *  Aufteilung der CV's:
 *  CV      Bedeutung    
 *  47      Kennung für Erstinitiierung, allgemeine Optionen die für den gesamten Decoder gelten
 *  48/49   Pom-Adresse
 *  50-54   Parameter für 1. Weichenadresse
 *  55-59   Parameter für 2. Weichenadresse
 *  ...
 *  Bedeutung der CV's bei den verschiedenen Funktione (CV-Nummern für 1. Weichenadresse)
 *  FSERVO Servo:
 *  CV50    Bit0 = 1: AutoOff der Servoimpulse bei Stillstand des Servo
 *          Bit1 = 1: 'Direct-Mode' auch während der Servobewegung wird auf einen erneuten
 *                    Stellbefehl reagiert, und gegebenenfalls sofort die Drehrichtung geändert
 *          Bit3 = 1: kein Überprüfung auf Servoposition bei Empfang eines DCC-Befehls
 *                    bei AUTOOFF und gleicher Position werden wieder Impulse ausgegeben
 *  CV51    Position des Servo für Weichenstellung '0' ( in Grad, 0...180 )
 *  CV52    Position des Servo für Weichenstellung '1' ( in Grad, 0...180 )
 *  CV53    Geschwindigkeit des Servo
 *  CV54    aktuelle Weichenstellung ( nicht manuell verändern! )
 *  
 *  FCOIL Doppelspulenantrieb: ( derzeit nur mit automatischer Abschaltung )
 *  CV50    Bit0 = 1: Spulenausgang nur automatisch abschalten
 *               = 0: Spulenausgang auch über DCC-Befehl abschalten
 *          Bit3 = 1: kein Überprüfung auf Weichenposition. Gegebenenfalls wird auch an gleichen
 *                    Anschluss wiederholt ein Puls ausgegeben
 *  CV51    Einschaltdauer der Spule  ( in 10ms Einheiten ) 
 *          0= keine automatische Abschaltung, Bit0 im Modebyte muss 0 sein
 *  CV52    minimale Ausschaltdauer der Spule ( in 10ms Einheiten )
 *  CV53    -
 *  CV54    aktuelle Weichenstellung ( nicht manuell verändern! )
 *  
 *  FSTATIC statischer/Blinkender Ausgang 
 *  CV50    Bit0 = 1: Blinken,  0: statisch
 *          Bit1 = 1: Beim Blinken starten erst beide Leds dann Wechselblinken
 *          Bit2 = 1: mit weichem Auf/Abblenden 
 *  CV51    Einschaltzeit des Blinkens ( 10ms Einheiten )
 *  CV52    Ausschaltzeit des Blinkens ( 10ms Einheiten )
 *  CV53    1. Einschaltzeit beim Start des Blinkens
 *  CV54    aktueller Zusatnd ( nicht manuell verändern! )
 *  
 *  FSIGNAL2 Lichtsignalfunktion mit 1..3 Weichenadressen 
 *          bei den Folgeadressen ist als Typ FSIGNAL0 einzutragen
 *          Lichtsignale starten beim Einschalten immer im Zustand 0 (Bitmuster CV51)
 *  CV50    Signalmodus: Bit7=1 : invertiert die Softled-Ausgänge (HIGH=OFF) (MobaTools ab V0.9)
 *          Bit 2..0: Bitmuster hard/soft gibt an, welche Ausgänge 'hart' umschalten (Bit=1)
 *          und welche Ausgänge weich überblenden (Bit=0)
 *  CV51    Bitmuster der Ausgänge für Befehl 1.Adresse 0 (rot)
 *  CV52    Bitmuster der Ausgänge für Befehl 1.Adresse 1 (grün)
 *  CV53    Index des Vorsignals am gleichen Mast ( 1 …. )
 *  CV54    Bitmuster der Zustände, bei denen das Vorsignal dunkel ist:
 *              Bit 0: Befehl 1.Adresse 0 (rot)
 *              Bit 1: Befehl 1.Adresse 1 (grün)
 *              Bit 2: Befehl 2.Adresse 0 (rot)
 *              u.s.w.
 *  FSIGNAL0 1. Folgeadresse (optional)
 *  CV55    Bit 2.. 0 Bitmuster hard/soft gibt an, welche Ausgänge 'hart' umschalten (Bit=1)
 *          und Welche Ausgänge weich überblenden (Bit=0)
 *  CV56    Bitmuster der Ausgänge für Befehl 2.Adresse 0 (rot)
 *  CV57    Bitmuster der Ausgänge für Befehl 2.Adresse 1 (grün)
 *  CV58    reserved
 *  CV59    reserved
 *  FSIGNAL0 2. Folgeadresse (optional)
 *  CV60     Bit 2.. 0 Bitmuster hard/soft gibt an, welche Ausgänge 'hart' umschalten (Bit=1)
 *           und Welche Ausgänge weich überblenden (Bit=0)
 *  CV61     Bitmuster der Ausgänge für Befehl 3.Adresse 0 (rot)
 *  CV62     Bitmuster der Ausgänge für Befehl 3.Adresse 1 (grün)
 *  CV63     reserved
 *  CV64     reserved
 *  
 *  FVORSIG Vorsignalfunktion
 *          weitgehend wie FSIGNAL2 ausser:
 *  CV53    low Byte der Adresse des angekündigten Hauptsignals
 *  CV54    high Byte der Adrsse des angekündigten Hauptsignals
  */
#define ENCODER_DOUBLE  // Eigenschaften des Drehencoders (Impulse per Raststellung)


// vom Anwender änderbare Parameter um den Zubehördecoder an die verwendete HW anzupassen

// Beispiel für Variante mit Licht-Ausfahrsignal mit Vorsignal, mit Betriebsmode Led an Pin 13 (interne Led)
// für Arduino Nano

//----------------------------------------------------------------
// Hardwareabhängige Konstante ( nicht per CV änderbar)
//----------------------------------------------------------------
const byte dccPin       =   2;
const byte ackPin       =   4;

// Eingänge analog: ( Bei Nano und Mini - Versionen kann hier auch A7 und A6 verwendet werden, um mehr
//                    digital nutzbare Ports freizubekommen.
//                    beim UNO sind A7+A6 nicht vorhanden! )
// #define FIXMODE NORMALMODE    // Ist dieses define aktiv, wird der Betriebsmode fest gesetzt, betrModeP wird dann
                        // nicht gelesen und ignoriert. Mögliche Werte:
                        // NORMALMODE, POMMODE, INIMODE, ADDRMODE
const byte betrModeP    =   A1;     // Analogeingang zur Bestimmung des Betriebsmodus. Wird nur beim
                                    // Programmstart eingelesen!
const byte resModeP     =   A0;     // Rücksetzen CV-Werte + Mittelstellung Servos

// Eingänge digital (die Ports A0-A5 lassen sich auch digital verwenden): ---------

// Drehencoder zur Servojustierung ...........
#define ENCODER_AKTIV       // Wird diese Zeile auskommentiert, wird der Encoder nicht verwendet. 
                            // Die Encoder-Ports werden dann ignoriert, und können anderweitig 
                            // verwendet werden.
const byte encode1P     =   1;     // Eingang Drehencoder zur Justierung.
const byte encode2P     =   0;
// ............................................
//-------------------------------------------------------------------------------------------------------
// Betriebswerte ( per CV änderbar ) Diese Daten werden nur im Initiierungsmodus in die CV's geschrieben.
// Der Initiierungsmodus lässt sich per Mode-Eingang aktivieren oder er ist automatisch aktiv, wenn keine
// sinnvollen Werte im CV47 stehen.
//-------------------------------------------------------------------------------------------------------
const byte DccAddr          =  20;    // DCC-Decoderadresse
const byte iniMode          = 0x50 | AUTOADDR /*| ROCOADDR*/;  // default-Betriebsmodus ( CV47 )
const int  PomAddr          = 50;    // Adresse für die Pom-Programmierung ( CV48/49 )

//Konstante für Lichtsignalfunktion
#define SIG_DARK_TIME   300     // Zeit zwischen Dunkelschalten und Aufblenden des neuen Signalbilds
#define SIG_RISETIME    500     // Auf/Abblendezeit

// Ausgänge:  mit NC gekennzeichnete Ausgänge werden keinem Port zugeordnet. Damit können Ports gespart werden,
//            z.B. wenn bei einem Servo kein Polarisierungsrelais benötigt wird
const byte modePin      =   17;     // Anzeige Betriebszustand (Normal/Programmierung) (Led)

#define COILMOD     NOPOSCHK|CAUTOOFF
#define SERVOMOD    SAUTOOFF|NOPOSCHK|SDIRECT
#define STATICMOD   CAUTOOFF|BLKSOFT|BLKSTRT    // Wechselblinker mit beiden Leds an beim Start            
const byte iniTyp[]     =   {    FSTATIC,  FSERVO,   FSIGNAL2,   FSIGNAL0,   FVORSIG,   FCOIL };
const byte out1Pins[]   =   {       A2,         3,   /*rt*/ 9,   /*rt*/10, /*ge*/15,        5 };  // output-pins der Funktionen
const byte out2Pins[]   =   {       A3,        NC,   /*gn*/14,   /*ws*/ 8, /*gn*/16,        6 };
const byte out3Pins[]   =   {       NC,        NC,   /*ge*/ 7,         NC,       NC,       NC };
 
const byte iniFmode[]     = {STATICMOD,  SERVOMOD,          0,          0,         0,  COILMOD };
const byte iniPar1[]      = {       50,        30,    0b01001,    0b10001,       0b01,       50 };
const byte iniPar2[]      = {       50,       150,    0b00010,    0b00110,       0b10,       50 };
const byte iniPar3[]      = {       50,         8,          5,          0,         19,        0 };
const byte iniPar4[]      = {        0,         0,    0b00101,          0,         0,        0,}; // nur für Lichtsignale!
//------------------------------------------------------------------------------------


