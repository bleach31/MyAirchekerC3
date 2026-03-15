    
//------------------------------------------------
/*
 * Sender動作時の送信先アドレス、チャンネル
*/

#define TARGET_ADDRESS 0xFFFF     // 0xFFFF is Broadcast (固定送信モード)
#define TARGET_CHANNEL 8


//-------------------------------------------------
/*
 *  E220-900T22S(JP) Registar Definition
*/

/* ---------- */
// ADDH/ADDL (Own Address)　      //=====  00H,01H  ADDH, ADDL
//

#define OWN_ADDRESS 0x0


/* ---------- */
// UART Serial Port Rate(bps)     //=====  02H      REG0      b7-b5
//
// define by  #define LoRa_BaudRate 9600 in esp32_e220900t22s_jp_lib.h
#define UART_BAUD_RATE 0b011     // for example


/* ---------- */
// Air Data Rate(bps)            //=====  02H      REG0      b4-b0
//

//#define AIR_DATA_RATE 0b00000 //  15,625  SF  5 /  BW 125
//#define AIR_DATA_RATE 0b00100 //   9,375  SF  6 /  BW 125
//#define AIR_DATA_RATE 0b01000 //   5,469  SF  7 /  BW 125
//#define AIR_DATA_RATE 0b01100 //   3,125  SF  8 /  BW 125
#define AIR_DATA_RATE 0b10000 //   1,758  SF  9 /  BW 125

//#define AIR_DATA_RATE 0b00001 //  31,250  SF  5 /  BW 250
//#define AIR_DATA_RATE 0b00101 //  18,750  SF  6 /  BW 250
//#define AIR_DATA_RATE 0b01001 //  10,938  SF  7 /  BW 250
//#define AIR_DATA_RATE 0b01101 //   6,250  SF  8 /  BW 250
//#define AIR_DATA_RATE 0b10001 //   3,516  SF  9 /  BW 250
//#define AIR_DATA_RATE 0b10101 //   1,953  SF 10 /  BW 250

//#define AIR_DATA_RATE 0b00010 //  62,500  SF  5 /  BW 500
//#define AIR_DATA_RATE 0b00110 //  37,500  SF  6 /  BW 500
//#define AIR_DATA_RATE 0b01010 //  21,875  SF  7 /  BW 500
//#define AIR_DATA_RATE 0b01110 //  12,500  SF  8 /  BW 500
//#define AIR_DATA_RATE 0b10010 //   7,031  SF  9 /  BW 500
//#define AIR_DATA_RATE 0b10110 //   3,906  SF 10 /  BW 500
//#define AIR_DATA_RATE 0b11010 //   2,148  SF 11 /  BW 500



/* ---------- */
// サブパケット長                   //===== 03H      REG1      b7,b6
//

//#define SUBPACKET_SIZE 0b00   // 200 byte
//#define SUBPACKET_SIZE 0b01   // 128 byte
//#define SUBPACKET_SIZE 0b10   //  64 byte
#define SUBPACKET_SIZE 0b11   //  32 byte



/* ---------- */
// RSSI環境ノイズの有効化           //=====  03H      REG1      b5
//

//#define AMBIENT_NOISE  0b0    // 無効
#define AMBIENT_NOISE  0b1    // 有効



/* ---------- */
// 送信出力電力                    //=====  03H      REG1      b1,b0
//

#define TRANSMITTING_PWR   0b01 // 13(+-1)
//#define TRANSMITTING_PWR   0b10 //  7
//#define TRANSMITTING_PWR   0b11 //  0



/* ---------- */
// 周波数チャンネルの指定(own channel)
//

#define OWN_CHANNEL 8            //=====  04H      REG2



/* ---------- */
// RSSIバイトの有効化             //===== 05H      REG3      b7
//

//#define RSSI_BYTE        0b0  // 無効
#define RSSI_BYTE        0b1  // 有効


/* ---------- */
// 送信方法（送信モード） 　       //=====  05H      REG3      b6
//

//#define TRANSMITTION_METHOD 0b0  // トランスペアレント送信モード
#define TRANSMITTION_METHOD 0b1  // 固定送信モード



/* ---------- */
// WORサイクル                  //=====  05H      REG3      b2-b0
//

//#define WOR_CYCLE    0b000   //  500
//#define WOR_CYCLE    0b001   // 1000
//#define WOR_CYCLE    0b010   // 1500
//#define WOR_CYCLE    0b011   // 2000
//#define WOR_CYCLE    0b100   // 2500
#define WOR_CYCLE    0b101   // 3000
//#define WOR_CYCLE    0b110   // 3500
//#define WOR_CYCLE    0b111   // 4000



/* ---------- */
// CRYPT_H/CRYPT_L　             //=====  06H,07H  CRYPT_H, CRYPT_L
//

#define ENCRYPTION_KEY  0x0000

/* ---------- */
// RESERVE BIT

#define RESERVE1 0b0
#define RESERVE2 0b0
