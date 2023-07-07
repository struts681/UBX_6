#include "ubx.c"

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
    NAV         = 0x01, //Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used
    RXM         = 0x02, //Receiver Manager Messages: Satellite Status, RTC Status
    INF         = 0x04, //Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice
    ACK         = 0x05, //Ack/Nack Messages: as replies to CFG Input Messages
    CFG         = 0x06, //Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc.
    MON         = 0x0A, //Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status
    AID         = 0x0B, //AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input
    TIM         = 0x0D, //Timing Messages: Timepulse Output, Timemark Results
    ESF         = 0x10, //External Sensor Fusion Messages: External sensor measurements and status information
}


//UBX message IDs
typedef enum _CLASS_NAV {
    AOPSTATUS   = 0x60, //AssistNow Autonomous Status
    CLOCK       = 0x22, //Clock Solution
    DGPS        = 0x31, //DGPS Data Used for NAV
    DOP         = 0x04, //Dilution of precision
    EKFSTATUS   = 0x40, //Dead Reckoning Software Status
    POSECEF     = 0x01, //Position Solution in ECEF
    POSLLH      = 0x02, //Geodetic Position Solution
    SBAS        = 0x32, //SBAS Status Data
    SOL         = 0x06, //Navigation Solution Information
    STATUS      = 0x03, //Receiver Navigation Status
    SVINFO      = 0x30, //Space Vehicle Information
    TIMEGPS     = 0x20, //GPS Time Solution
    TIMEUTC     = 0x21, //UTC Time Solution
    VELECEF     = 0x11, //Velocity Solution in ECEF
    VELNED      = 0x12, //Velocity Solution in NED
}

typedef enum _CLASS_RXM {
    ALM         = 0x30, //GPS Constellation Almanach (poll/io message depending on length)
    EPH         = 0x31, //GPS Constellation Ephemeris Data (poll/io message depending on length)
    PMREQ       = 0x41, //Requests a Power Management task
    RAW         = 0x10, //Raw Measurement Data
    SFRB        = 0x11, //Subframe Buffer
    SVSI        = 0x20, //SV Status Info
}

typedef enum _CLASS_INF {
    DEBUG       = 0x04, //ASCII String output, indicating debug output
    ERROR       = 0x00, //ASCII String output, indicating an error
    NOTICE      = 0x02, //ASCII String output, with informational contents
    TEST        = 0x03, //ASCII String output, indicating test output
    WARNING     = 0x01, //ASCII String output, indicating a warning
}

typedef enum _CLASS_ACK {
    ACK_ACK     = 0x01, //Message Acknowledged
    ACK_NAK     = 0x00, //Message Not-Acknowledged
}

//Most of these with a length of 0 will poll the receiver for their current settings
typedef enum _CLASS_CFG {
    ANT         = 0x13, //Antenna Control Settings
    CFG         = 0x09, //Clear, Save and Load configurations command
    DAT         = 0x06, //Datum Settings(dependent on length)
    EKF         = 0x12, //EKF Module Settings
    ESFGWT      = 0x29, //Set/Get settings of gyro+wheel tick sol (GWT) - LEA-6R
    FXN         = 0x0E, //FXN configuration
    INF         = 0x02, //Information message configuration
    ITFM        = 0x39, //Jamming/Interference Monitor configuration.
    MSG         = 0x01, //Message configuration(dependent on length)
    NAV5        = 0x24, //Navigation Engine Settings
    NAVX5       = 0x23, //Navigation Engine Expert Settings
    NMEA        = 0x17, //NMEA protocol configuration
    NVS         = 0x22, //Clear, Save and Load non-volatile storage data
    PM2         = 0x3B, //Extended Power Management configuration
    PM          = 0x32, //Power Management configuration
    PRT         = 0x00, //I/O Port configuration(dependent on length)
    RATE        = 0x08, //Navigation/Measurement Rate Settings
    RINV        = 0x34, //Set/Get contents of Remote Inventory
    RST         = 0x04, //Reset Receiver command
    RXM         = 0x11, //RXM configuration
    SBAS        = 0x16, //SBAS configuration
    TMODE2      = 0x3D, //Time Mode Settings 2
    TMODE       = 0x1D, //Time Mode Settings
    TP5         = 0x31, //Set/Get TimePulse Parameters
    TP          = 0x07, //Set/Get TimePulse Parameters (not sure how these two differ)
    USB         = 0x1B, //Set/Get USB configuration
}

typedef enum _CLASS_MON {
    HW2         = 0x0B, //Extended Hardware Status
    HW          = 0x09, //Hardware Status
    IO          = 0x02, //I/O Subsystem Status
    MSGPP       = 0x06, //Message Parse and Process Status
    RXBUF       = 0x07, //Receiver Buffer Status
    RXR         = 0x21, //Receiver Status Information
    TXBUF       = 0x08, //Transmitter Buffer Status
    VER         = 0x04, //Receiver/Software/ROM Version
}

typedef enum _CLASS_AID {
    ALM         = 0x30, //GPS Aiding Almanac Data features
    ALPSRV      = 0x32, //AlmanacPlus features
    ALP         = 0x50, //Data transfer
    AOP         = 0x33, //AssistNow Autonomous data
    DATA        = 0x10, //Polls all GPS Initial Aiding Data
    EPH         = 0x31, //GPS Aiding Ephemeris Input/Output Message
    HUI         = 0x02, //GPS Health, UTC, ionosphere parameters
    INI         = 0x01, //GPS Aiding data, time, frequency, clock drift
    REQ         = 0x00, //Sends a poll(AID-DATA) for all GPS Aiding Data
}

typedef enum _CLASS_TIM {
    SVIN        = 0x04, //Survey-in data
    TM2         = 0x03, //Time mark data
    TP          = 0x01, //Timepulse Timedata
    VRFY        = 0x06, //Sourced Time Verification
}

typedef enum _CLASS_ESF {
    MEAS        = 0x02, //External Sensor Fusion Measurements (LEA-6R)
    STATUS      = 0x10, //Sensor Fusion Status Information (LEA-6R)
}
const static unsigned char _sync[2] = {0xB5, 0x62};
static unsigned char _class;
static unsigned char _id;
static uint16_t _len;
static unsigned char _ck_a;
static unsigned char _ck_b;


