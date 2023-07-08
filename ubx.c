#include "ubx.h"

void gps_poll(unsigned char _class, unsigned char _id) {
    char poll_message[9];

    poll_message[0] = _sync[0];
    poll_message[1] = _sync[1];
    poll_message[2] = _class;
    poll_message[3] = _id;
    poll_message[4] = '0';
    poll_message[5] = '0';
    poll_message[6] = _class + _id;
    poll_message[7] = _class + _id;
    poll_message[8] = '\0';

    //sprintf("%c%c%c%c00%c%c", _sync[0], _sync[1], _class, _id, _class + _id, _class + _id);
    uart_puts(uart0, poll_message);

}


uint8_t _generate_checksum(unsigned char *_str) {
    uint8_t check = 0;
    uint16_t i;
    uint16_t length = strlen(_str);

    for(i = 0; _str[i] != '\0'; i++)
    {
        check += _str[i];
    }
}
