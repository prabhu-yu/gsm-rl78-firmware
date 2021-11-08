#!/usr/bin/python3.8

import socket
IP = "127.0.0.1"
PORT = 5555

#typedef enum {
off= 0
on= 1
false= 0
true= 1
bool_invalid= 2
#} bool_t;

#typedef enum{
manual_switch = 0
fixed_timer_switch = 1
astro_timer_switch = 2
motor_starter = 3
water_sensor_tanks_1 = 4
motor_starter_water_sensor_tanks_1 = 5
motor_starter_water_sensor_tanks_2 = 6
school_timer = 7
invalid_max_device_mode = 8
#} device_type_t;

#typedef enum {
starter_mode_automatic = 0
starter_mode_temporary = 1
starter_mode_timer     = 2
starter_mode_duration  = 3
starter_mode_invalid   = 4
#}starter_mode_t;

#typedef enum {
cmd_write_device_type                          = 11#//cmd*device_type_t
cmd_get_status                                 = 12#//dump all prov sturct in 1 or two sms in NAME:VALUE pair
cmd_reset_mcu                                  = 13
cmd_reset_prov                                 = 31985
cmd_set_mcu_boot_delay                         = 14
cmd_set_date_time                              = 15
cmd_write_time_zone                            = 16
cmd_write_password                             = 17
cmd_write_sms_reply_enabled                    = 18
cmd_read_sw_hw_rev                             = 19
# WATER LEVEL RELATED-------------------------------------------
cmd_write_water_level_soak_time                = 30#// cmd*WATER_LEVEL_SOAK_SECONDS
cmd_write_water_level_max_level                = 31#// cmd*WATER_LEVEL_MAX_LEVEL
#phone number RELATED------------------------------------------
cmd_write_add_phone_number                     = 50#//cmd*index*DEBUG_NUMBER
cmd_write_delete_phone_number                  = 51#//cmd*index
cmd_read_phone_numbers                         = 52
#Fixed Timer related------------------------------------------
cmd_write_light_24h_timer_table                = 60#//cmd*index*hh*mm*ss*rly_status
cmd_write_light_24h_timer_table_prov_max       = 62#//cmd*prov_max
cmd_read_light_24h_timer_table                 = 63
#// Motor Related-----------------------------------------------
cmd_write_starter_mode                         = 70#// cmd*motor_N*starter_mode_t
cmd_write_starter_status                       = 71#// cmd*motor_N*0|1 on/off
cmd_write_starter_n_call_aware                 = 72#// cmd*motor_N
cmd_write_starter_start_stop_relay_hold_time   = 73#// cmd*start_hold_sec*stop_hold_sec
#//Manual switch related------------------------------------------
cmd_write_manual_switch_status_provision       = 80#//cmd*switch_No*off_on
#// Astro Timer related------------------------------------------
cmd_write_lat_long                             = 90#//cmd*lattitude*longitude
cmd_write_sun_elevation_rise_set               = 91#//cmd*sun_elevation_rise*sun_elevation_set
cmd_read_astro_prov                            = 92
#// Last value- keep updated------------------------------------------
cmd_max                                        = 100
#} text_command_t ;

device_type = 3
clcc_iteration = 0
cclk_iteration = 0

def respond(new_socket, text):
    print(f'respond = {text}')
    new_socket.sendall(text)
    return

# sms prov testing
def cmgl_yield_sms():
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*50*0*DEBUG_NUMBER\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    device_type = 0
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*11*{device_type}\r\nOK\r\n'
    b = s.encode('ASCII')
    device_type += 1
    yield b

    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*11*{device_type}\r\nOK\r\n'
    b = s.encode('ASCII')
    device_type += 1
    yield b

    return '\r\nOK\r\n'

def cmgl_yield_water_level():
    '''s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*50*0*DEBUG_NUMBER\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    device_type = 4 #water_sensor_tanks_1
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*11*{device_type}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    soak_seconds = 3
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*30*{soak_seconds}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    max_levels = 8
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*31*{max_levels}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*12\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b'''

    for i in range(1000):
        b = b'\r\nOK\r\n'
        yield b

    return '\r\nOK\r\n'

def cmgl_yield_astro():
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*11*{astro_timer_switch}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    lattitude= 15 + 11/60
    longitude= 75 + 44/60
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*90*{lattitude}*{longitude}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    rise = -6
    set = -6
    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*91*{rise}*{set}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    for i in range(100000):
        b = b'\r\nOK\r\n'
        yield b

    return '\r\nOK\r\n'

def cmgl_yield_motor_starter_automatic():
    b = b'\r\nOK\r\n'
    yield b

    s = f'+CMGL: 1,"REC READ",DEBUG_NUMBER,"","07/05/01,08:00:15+22"\r\n108*11*{motor_starter}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    start_hold= 3
    stop_hold= 4
    s = f'+CMGL: 1,"REC READ","+DEBUG_NUMBER","","07/05/01,08:00:15+22"\r\n108*{cmd_write_starter_start_stop_relay_hold_time}*{start_hold}*{stop_hold}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    starter_number=0
    s = f'+CMGL: 1,"REC READ","+DEBUG_NUMBER","","07/05/01,08:00:15+22"\r\n108*{cmd_write_starter_mode}*{starter_number}*{starter_mode_temporary}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    starter_number=0
    start_status= on
    s = f'+CMGL: 1,"REC READ","+DEBUG_NUMBER","","07/05/01,08:00:15+22"\r\n108*{cmd_write_starter_status}*{starter_number}*{start_status}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    starter_number=0
    start_status= off
    s = f'+CMGL: 1,"REC READ","+DEBUG_NUMBER","","07/05/01,08:00:15+22"\r\n108*{cmd_write_starter_status}*{starter_number}*{start_status}\r\nOK\r\n'
    b = s.encode('ASCII')
    yield b

    for i in range(100000):
        b = b'\r\nOK\r\n'
        yield b

    return '\r\nOK\r\n'

def clcc_motor():
    s = b'+CLCC: 1,0,0,0,0,"DEBUG_NUMBER",129,"",4' + b'\r\nOK\r\n'
    b = s.encode('ASCII')
    return b

def cclk_yield():
    print("cclk iter 1")
    time = '"21/02/06,00:00:00+30"'
    yield time

    print("cclk iter 2")
    time = '"21/02/06,00:00:00+30"'
    yield time

    print("cclk iter 3")
    time = '"21/02/06,00:00:00+30"'
    yield time

    print("cclk iter 4")
    time = '"21/02/06,00:00:00+30"'
    yield time

    print("cclk iter 5")
    time = '"21/02/06,00:00:00+30"'
    yield time

    print("cclk iter 6")
    time = '"21/02/06,03:15:18+30"'
    yield time

    print("cclk iter 7")
    time = '"21/02/06,06:15:18+30"'
    yield time

    print("cclk iter 8")
    time = '"21/02/06,18:15:18+30"'
    yield time

    print("cclk iter 9")
    time = '"21/02/06,23:15:18+30"'
    yield time

    time = '"21/02/07,00:15:18+30"'
    yield time

    for i in range(100000):
        time = '"21/02/07,01:02:03+30"'
        yield time
##########################

def handle_at_command(new_socket, command):
    global device_type
    global clcc_iteration
    global cclk_iteration
    global sms_iter_obj
    global cclk_iter_obj

    ok=b'\r\nOK\r\n'

    print(f'command = {command}', end='')
    if -1 != command.find(b'AT\r'):
        print(' === AT')
        respond(new_socket, ok)
        return

    if -1 != command.find(b'AT+CSQ'):
        print(' === CSQ')
        respond(new_socket, b'+CSQ:27,99' + ok)
        return

    if -1 != command.find(b'AT+CMGF'):
        print(' === CMGF')
        respond(new_socket, ok)
        return

    if -1 != command.find(b'AT+CLIP'):
        print(' === CLIP')
        respond(new_socket, ok)
        return

    if -1 != command.find(b'AT+CPMS'):
        respond(new_socket, ok)
        print(' === CPMS')
        return

    if -1 != command.find(b'AT+CCLK'):
        time = next(cclk_iter_obj)
        s = f'+CCLK: {time}\r\nOK\r\n'
        s = s.encode('ASCII')
        respond(new_socket, s)
        print('CCLK ==> {s}')
        return

    if -1 != command.find(b'AT+CMGL'):
        s = next(sms_iter_obj)
        print(f'CMGL:')
        respond(new_socket, s)
        return

    if -1 != command.find(b'AT+CLCC'):
        clcc_iteration += 1
        print(f'clcc_iteration = {clcc_iteration}')
        if clcc_iteration >= 20:
            respond(new_socket, b'+CLCC: 1,0,0,0,0,"DEBUG_NUMBER",129,"",4' + b'\r\nOK\r\n')
        else:
            respond(new_socket, ok)

        return

    if -1 != command.find(b'ATH\r'):
        print('ATH')
        respond(new_socket, ok)
        return

    if -1 != command.find(b'AT+CMGD'):
        print('CMGD')
        respond(new_socket, ok)
        return

    if -1 != command.find(b'AT+CHLD=0'):
        print('CHLD')
        respond(new_socket, ok)
        return

    if -1 != command.find(b'AT+CMGW'):
        print('AT+CMGW')
        respond(new_socket, b"\r\n> ")
        return

    if -1 != command.find(b'\r\x1A\r'):
        print(f'CMGW+ctrl-Z')
        respond(new_socket, b"+CMGW:1\r\nOK\r\n")
        return

    if -1 != command.find(b'AT+CMSS'):
        print('AT+CMSS')
        respond(new_socket, b"\r\n+CMSS:1\r\nOK\r\n")
        return

    if -1 != command.find(b'AT+CSQ'):
        print('')
        return

    print('unknown Command')
    return


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1);
sock.bind( (IP, PORT) )

sms_iter_obj = cmgl_yield_water_level()
sms_iter_obj = cmgl_yield_astro()
sms_iter_obj = cmgl_yield_motor_starter_automatic()

cclk_iter_obj = cclk_yield()

while True:
    sock.listen()
    new_socket, addr = sock.accept()
    print(f'got a connection : {addr}')
    while True:
        data = new_socket.recv(1024)
        if not data:
            print('new_socket closed! listning again')
            break
        handle_at_command(new_socket, data)

    new_socket.close()
