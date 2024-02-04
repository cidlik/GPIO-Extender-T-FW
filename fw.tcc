/*
PA4 - 1
PA2 - 2
PA0 - 3
PA7 - 4
PA6 - 5

PA3 - 6
PA1 - 7
PA13 - 8
PB1 - 9
PA14 - 10

PF0 - LED Green
PF1 - LED Red (do not use it)
*/

#define DEBUG 0

#define BUFFER_LEN 8
#define PIN_COUNT  5

#define PIN_UP    '1'
#define PIN_DOWN  '0'

#define FW_INFO "GPIO Extender-T (TOIC) v.0.1\r\n"

var out_pin_val = [PA4.VALUE, PA2.VALUE, PA0.VALUE, PA7.VALUE, PA6.VALUE];
var out_pin_mode = [PA4.MODE, PA2.MODE, PA0.MODE, PA7.MODE, PA6.MODE];
var in_pin_mode = [PA3.MODE, PA1.MODE, PA13.MODE, PB1.MODE, PA5.MODE];


var get_in(var pin) {
    switch (pin) {
        case 6:
            return PA3.VALUE;
        case 7:
            return PA1.VALUE;
        case 8:
            return PA13.VALUE;
        case 9:
            return PB1.VALUE;
        case 10:
            return PA5.VALUE;
        default:
            return 255;
    }
}


var chr(var u32) {
    return u32 & 0xff;
}


var command_A(var arr) {
    var pin1;
    var pin2;
    var pin3;
    var pin4;
    var pin5;

    pin1 = get_in(6);
    pin2 = get_in(7);
    pin3 = get_in(8);
    pin4 = get_in(9);
    pin5 = get_in(10);

    sprintf(&UART0.TX, "~A%d%d%d%d%d", pin1, pin2, pin3, pin4, pin5);
}


var command_G(var arr) {
    var pin = (MSG.SIZE == 3) ? chr(arr[2]) - 48 : 10;
    sprintf(&UART0.TX, "~G%d%d", pin, get_in(pin));
}


var command_P(var arr) {
    var i;
    
    for (i = 0; i < PIN_COUNT; i++) {
        out_pin_val[i] = (chr(arr[2 + i]) == PIN_UP) ? 1: 0;
    }
    sprintf(&UART0.TX, "~P%s", SYS.RAM + 2);
}


var command_S_R(var arr, var state) {
    var pin;

    pin = chr(arr[2]) - 48;
    if ((pin < 1) || (pin > PIN_COUNT)) {
        return 1;
    }
#if DEBUG
    sprintf(&UART0.TX, "pin: %d; state: %c\r\n", pin, state);
#endif
    out_pin_val[pin - 1] = (state == PIN_UP) ? 1: 0;
    sprintf(&UART0.TX, "~%c%d", (state == PIN_UP) ? 'S': 'R', pin + 1);
}


var command_handler(var arr) {
    switch (chr(arr[1])) {
        case 'S':
            command_S_R(arr, PIN_UP);
            break;
        case 'R':
            command_S_R(arr, PIN_DOWN);
            break;
        case 'G':
            command_G(arr);
            break;
        case 'A':
            command_A(arr);
            break;
        case 'P':
            command_P(arr);
            break;
        case 'B':
            SYS.RESET = 1;
            break;
        case 'I':
            sprintf(&UART0.TX, "%s\r\n", FW_INFO);
        default:
            break;
    }
}


var _msg() {
    var arr = SYS.RAM;
    var i;

    memset(SYS.RAM, 0, BUFFER_LEN);
    memcpy(SYS.RAM, &MSG.RX, MSG.SIZE);
    
#if DEBUG
    sprintf(&UART0.TX, "MSG.SIZE: %d\r\n", MSG.SIZE);
    for (i = 0; i < MSG.SIZE; i++) {
        sprintf(&UART0.TX, "Char %d: %c = %d\r\n", i, arr[i], chr(arr[i]));
    }
#endif

    if (chr(arr[0]) == '~') {
        command_handler(arr);
    }
}


var pin_setup() {
    var i;

    /* 1-5 pins to output */
    for (i = 0; i < PIN_COUNT; i++) {
        out_pin_mode[i] = GPIO_MODE_OUTPUT|GPIO_INIT_LOW|GPIO_OTYPE_PP;
    }

    /* 6-10 pins to input */
    for (i = 0; i < PIN_COUNT; i++) {
        in_pin_mode[i] = GPIO_MODE_INPUT|GPIO_PULL_UP;    
    }
}

var uart_setup() {
    UART0.CFG = UART_MODE_PLAIN | UART_CONFIG_START;
    UART0.BAUD = 115200;
    UART0.EN = 1;
    __enable_irq();
}

var setup() {
    pin_setup();
    uart_setup();
}

var main() {
    setup();

    while (1){};
    return 0;
}
