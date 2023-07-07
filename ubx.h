#include <string.h>
#include <stdint.h>
#include "hardware/uart.h"

/*
 * packet anatomy:
 *
 * [bytes]
 * size:   |   1   |   1   |   1   |   1   |    2    |       ??        |   1   |   1   |
 * data:   |  SYNC |  SYNC | CLASS |   ID  |  LENGTH |     PAYLOAD     | CK_A  | CK_B  |
 *
 * *first two bytes(sync) are always 0xB5 0x62
 *
 *
 */

//UBX message class IDs
typedef enum _MSG_CLASS {
    CLASS_NAV         = 0x01, //Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used
    CLASS_RXM         = 0x02, //Receiver Manager Messages: Satellite Status, RTC Status
    CLASS_INF         = 0x04, //Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice
    CLASS_ACK         = 0x05, //Ack/Nack Messages: as replies to CFG Input Messages
    CLASS_CFG         = 0x06, //Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc.
    CLASS_MON         = 0x0A, //Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status
    CLASS_AID         = 0x0B, //AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input
    CLASS_TIM         = 0x0D, //Timing Messages: Timepulse Output, Timemark Results
    CLASS_ESF         = 0x10, //External Sensor Fusion Messages: External sensor measurements and status information
} UBX_MSG_CLASS;


//UBX message IDs
typedef enum _CLASS_NAV {
    NAV_AOPSTATUS   = 0x60, //AssistNow Autonomous Status
    NAV_CLOCK       = 0x22, //Clock Solution
    NAV_DGPS        = 0x31, //DGPS Data Used for NAV
    NAV_DOP         = 0x04, //Dilution of precision
    NAV_EKFSTATUS   = 0x40, //Dead Reckoning Software Status
    NAV_POSECEF     = 0x01, //Position Solution in ECEF
    NAV_POSLLH      = 0x02, //Geodetic Position Solution
    NAV_SBAS        = 0x32, //SBAS Status Data
    NAV_SOL         = 0x06, //Navigation Solution Information
    NAV_STATUS      = 0x03, //Receiver Navigation Status
    NAV_SVINFO      = 0x30, //Space Vehicle Information
    NAV_TIMEGPS     = 0x20, //GPS Time Solution
    NAV_TIMEUTC     = 0x21, //UTC Time Solution
    NAV_VELECEF     = 0x11, //Velocity Solution in ECEF
    NAV_VELNED      = 0x12, //Velocity Solution in NED
} UBX_CLASS_NAV;

typedef enum _CLASS_RXM {
    RXM_ALM         = 0x30, //GPS Constellation Almanach (poll/io message depending on length)
    RXM_EPH         = 0x31, //GPS Constellation Ephemeris Data (poll/io message depending on length)
    RXM_PMREQ       = 0x41, //Requests a Power Management task
    RXM_RAW         = 0x10, //Raw Measurement Data
    RXM_SFRB        = 0x11, //Subframe Buffer
    RXM_SVSI        = 0x20, //SV Status Info
} UBX_CLASS_RXM;

typedef enum _CLASS_INF {
    INF_DEBUG       = 0x04, //ASCII String output, indicating debug output
    INF_ERROR       = 0x00, //ASCII String output, indicating an error
    INF_NOTICE      = 0x02, //ASCII String output, with informational contents
    INF_TEST        = 0x03, //ASCII String output, indicating test output
    INF_WARNING     = 0x01, //ASCII String output, indicating a warning
} UBX_CLASS_INF;

typedef enum _CLASS_ACK {
    ACK_ACK     = 0x01, //Message Acknowledged
    ACK_NAK     = 0x00, //Message Not-Acknowledged
} UBX_CLASS_ACK;

//Most of these with a length of 0 will poll the receiver for their current settings
typedef enum _CLASS_CFG {
    CFG_ANT         = 0x13, //Antenna Control Settings
    CFG_CFG         = 0x09, //Clear, Save and Load configurations command
    CFG_DAT         = 0x06, //Datum Settings(dependent on length)
    CFG_EKF         = 0x12, //EKF Module Settings
    CFG_ESFGWT      = 0x29, //Set/Get settings of gyro+wheel tick sol (GWT) - LEA-6R
    CFG_FXN         = 0x0E, //FXN configuration
    CFG_INF         = 0x02, //Information message configuration
    CFG_ITFM        = 0x39, //Jamming/Interference Monitor configuration.
    CFG_MSG         = 0x01, //Message configuration(dependent on length)
    CFG_NAV5        = 0x24, //Navigation Engine Settings
    CFG_NAVX5       = 0x23, //Navigation Engine Expert Settings
    CFG_NMEA        = 0x17, //NMEA protocol configuration
    CFG_NVS         = 0x22, //Clear, Save and Load non-volatile storage data
    CFG_PM2         = 0x3B, //Extended Power Management configuration
    CFG_PM          = 0x32, //Power Management configuration
    CFG_PRT         = 0x00, //I/O Port configuration(dependent on length)
    CFG_RATE        = 0x08, //Navigation/Measurement Rate Settings
    CFG_RINV        = 0x34, //Set/Get contents of Remote Inventory
    CFG_RST         = 0x04, //Reset Receiver command
    CFG_RXM         = 0x11, //RXM configuration
    CFG_CFG_SBAS        = 0x16, //SBAS configuration
    CFG_TMODE2      = 0x3D, //Time Mode Settings 2
    CFG_TMODE       = 0x1D, //Time Mode Settings
    CFG_TP5         = 0x31, //Set/Get TimePulse Parameters
    CFG_TP          = 0x07, //Set/Get TimePulse Parameters (not sure how these two differ)
    CFG_USB         = 0x1B, //Set/Get USB configuration
} UBX_CLASS_CFG;

typedef enum _CLASS_MON {
    MON_HW2         = 0x0B, //Extended Hardware Status
    MON_HW          = 0x09, //Hardware Status
    MON_IO          = 0x02, //I/O Subsystem Status
    MON_MSGPP       = 0x06, //Message Parse and Process Status
    MON_RXBUF       = 0x07, //Receiver Buffer Status
    MON_RXR         = 0x21, //Receiver Status Information
    MON_TXBUF       = 0x08, //Transmitter Buffer Status
    MON_VER         = 0x04, //Receiver/Software/ROM Version
} UBX_CLASS_MON;

typedef enum _CLASS_AID {
    AID_ALM         = 0x30, //GPS Aiding Almanac Data features
    AID_ALPSRV      = 0x32, //AlmanacPlus features
    AID_ALP         = 0x50, //Data transfer
    AID_AOP         = 0x33, //AssistNow Autonomous data
    AID_DATA        = 0x10, //Polls all GPS Initial Aiding Data
    AID_EPH         = 0x31, //GPS Aiding Ephemeris Input/Output Message
    AID_HUI         = 0x02, //GPS Health, UTC, ionosphere parameters
    AID_INI         = 0x01, //GPS Aiding data, time, frequency, clock drift
    AID_REQ         = 0x00, //Sends a poll(AID-DATA) for all GPS Aiding Data
} UBX_CLASS_AID;

typedef enum _CLASS_TIM {
    TIM_SVIN        = 0x04, //Survey-in data
    TIM_TM2         = 0x03, //Time mark data
    TIM_TP          = 0x01, //Timepulse Timedata
    TIM_VRFY        = 0x06, //Sourced Time Verification
} UBX_CLASS_TIM;

typedef enum _CLASS_ESF {
    ESF_MEAS        = 0x02, //External Sensor Fusion Measurements (LEA-6R)
    ESF_STATUS      = 0x10, //Sensor Fusion Status Information (LEA-6R)
} UBX_CLASS_ESF;
const static unsigned char _sync[2] = {0xB5, 0x62};
void gps_poll(unsigned char _class, unsigned char _id);
