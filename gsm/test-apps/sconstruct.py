src_files = Split('''
            0-sms-util.c
            sms-main.c
            ''')

Program('exe', src_files)

