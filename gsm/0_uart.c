#include "0_all.h"

volatile unsigned int g_Uart0TxEnd;
volatile unsigned int g_Uart1TxEnd;
volatile unsigned int g_Uart2TxEnd;
volatile unsigned int g_Uart0RxEnd;
volatile unsigned int g_Uart1RxEnd;
volatile unsigned int g_Uart2RxEnd;
volatile int printf_uart_0_1;

#if MACHINE == UBUNTU
void create_debug_thread(void);
typedef struct {
    uint8_t *rx_buf;
    uint16_t rx_num;
    int present_rx_index; // -1 means, empty
    pthread_mutex_t mutexsum;
} thread_input_t;
#define SA struct sockaddr
thread_input_t thread_input[2]; // guard this by a lock
int sock_fd_at_server;
#endif

void read_uart0(uint8_t *buffer, int buffer_size, int sleep_time) {
    printf("Enter: read_uart0(buffer_size=%d, sleep_time=%d)\n", buffer_size, sleep_time);
    memset(buffer, 0, buffer_size);
    //debug_input_over = 1;
    R_UART0_Receive(buffer, buffer_size);
    while (sleep_time) {
        printf("sleep_ms_time = %d", sleep_time);
        sleep_time--;
        sleep(1); // Give some time so that user will enter the input
#if MACHINE == UBUNTU
        pthread_mutex_lock (&thread_input[0].mutexsum);
        if (thread_input[0].present_rx_index == (buffer_size-1)) {
            sleep_time = 0; // so can get out
        }
        pthread_mutex_unlock (&thread_input[0].mutexsum);
#endif
    }
    printf("Return read_uart0\n");
}


/*
* function to reverse a string
*/
void my_reverse(char str[], int len)
{
    int start, end;
    char temp;
    for(start=0, end=len-1; start < end; start++, end--) {
        temp = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = temp;
    }
}

// number to c string
char* my_itoa(int num, char* str, int base)
{
    int i = 0;
    int isNegative = 0;

    /* A zero is same "0" string in all base */
    if (num == 0) {
        str[i] = '0';
        str[i + 1] = '\0';
        return str;
    }

    /* negative numbers are only handled if base is 10
       otherwise considered unsigned number */
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
        num = num/base;
    }

    /* Append negative sign for negative numbers */
    if (isNegative){
        str[i++] = '-';
    }

    str[i] = '\0';
    my_reverse(str, i);
    return str;
}

#if MACHINE == RL78 
int __far putchar(int c)
{
	//uint8_t *str[1] = c;
	//sleep_ms(1);
	if(printf_uart_0_1){
		R_UART1_Send_blocking( (uint8_t *)&c, 1); // gsm
	}else{
		R_UART0_Send_blocking( (uint8_t *)&c, 1); // debug port
	}
	return c;
}

int __far getchar(void)
{
	uint8_t c;
	if(printf_uart_0_1){
		R_UART1_Receive_blocking(&c, 1);
	} else {
		R_UART0_Receive_blocking(&c, 1);
	}
	return (int)c;
}
#endif

int debug_socket;
void init_uart(void) {
#if MACHINE == RL78
	R_UART0_Start();
	R_UART1_Start();	
#elif MACHINE == UBUNTU
    printf(">>> init_uart\n");

    pthread_mutex_init(&thread_input[0].mutexsum, NULL);
    pthread_mutex_init(&thread_input[1].mutexsum, NULL);

    debug_socket = create_debug_socket();    
    create_debug_thread();
    create_at_server_thread();
    printf("<<< init_uart\n");
#else    
#endif
}

#if MACHINE == UBUNTU
unsigned int is_fd_readable(void) {

  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

MD_STATUS R_UART0_Send(uint8_t * const tx_buf, uint16_t tx_num) { 
    int i;
    for (i = 0; i< tx_num; i++) {
        printf("%c", tx_buf[i]);
    }
    g_Uart0TxEnd = 1;
}

MD_STATUS R_UART1_Send(uint8_t * const tx_buf, uint16_t tx_num) {
    int i;
    for (i = 0; i< tx_num; i++) {
        printf("%c", tx_buf[i]);
    }
    g_Uart0TxEnd = 1;
}

unsigned int uart0_byte_count; 
unsigned int uart1_byte_count;

#define UDP_PORT_DEBUG 5555
#define TCP_PORT_AT_SERVER 5555
int create_debug_socket(void) {
    int sockfd; 
    char buffer[1024]; 
    struct sockaddr_in servaddr; 
    printf("Enter : create_debug_socket\n");
    
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));      
    memset(&servaddr, 0, sizeof(servaddr));      
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(UDP_PORT_DEBUG); 
      
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    printf("Return : create_debug_socket\n");
    return sockfd;
}

void *at_command_rx_start_routine(void *vargp) {
    ssize_t read_bytes;
    unsigned int iteration=0;
    int bytes_read;

    while(1) {
        fflush(NULL);
        sleep_ms(1);
        pthread_mutex_lock (&thread_input[1].mutexsum);
        if(thread_input[0].present_rx_index < thread_input[0].rx_num) {
            bytes_read = recv (sock_fd_at_server, 
                    &thread_input[1].rx_buf[thread_input[1].present_rx_index + 1],
                    1, MSG_DONTWAIT);
            if (bytes_read > 0 ) {
                thread_input[1].present_rx_index += 1;
                char c = thread_input[1].rx_buf[thread_input[1].present_rx_index];
                //printf("\n buffer[%d] = ", thread_input[1].present_rx_index);
                if ('\r' == c ) {
                    printf("<r>");
                } else if ('\n' == c ) {
                    printf("<n>");
                }else {
                    printf("%c", c);
                }
            }
        } else {
            //printf("OF");
        }
        pthread_mutex_unlock(&thread_input[1].mutexsum);
    }
    return NULL; 
}

int create_at_server_thread(void) {
    char buffer[80]; 
    char* message = "Hello Server"; 
    struct sockaddr_in servaddr;  
    int n, len;
    int ret_val;
    sock_fd_at_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd_at_server < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    } else {
        printf("socket created \n");
    }  
    memset(&servaddr, 0, sizeof(servaddr));   
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(TCP_PORT_AT_SERVER);
    if(inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr)<=0) { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    }
  
    ret_val = connect(sock_fd_at_server, (struct sockaddr*)&servaddr,sizeof(servaddr));
    if (ret_val < 0) { 
        printf("\n Error : Connect Failed \n");
        exit(0);
    } else {
        printf("\n connect ok"); 
    }
  
#if 0    
    memset(buffer, 0, sizeof(buffer)); 
    strcpy(buffer, "Hello Server"); 

    while(1) {
        sleep(1);
        printf("Message from server: "); 
        read(sock_fd_at_server, buffer, sizeof(buffer)); 
        puts(buffer);
    }
#endif
    pthread_t thread_id2;
    pthread_create(&thread_id2, NULL, at_command_rx_start_routine, NULL);
}

void *debug_rx_start_routine(void *vargp) {
    ssize_t read_bytes;
    unsigned int iteration=0;
    int bytes_read;
    while(1) {
        fflush(NULL);
        sleep_ms(1000);
        pthread_mutex_lock (&thread_input[0].mutexsum);
        if(thread_input[0].present_rx_index < thread_input[0].rx_num) {
            int bytes_read = recvfrom(debug_socket, &thread_input[0].rx_buf[thread_input[0].present_rx_index + 1], 100, MSG_DONTWAIT, NULL, NULL);
            if (bytes_read > 0 ) {
                thread_input[0].present_rx_index += bytes_read;
                printf("buffer[%d] = %c, bytes_read=%d full = %s\n", thread_input[0].present_rx_index, 
                        thread_input[0].rx_buf[thread_input[0].present_rx_index], bytes_read, thread_input[0].rx_buf);
            } else {
                //printf("NO");
            }
        } else {
            printf("over flowing\n");
        }
        pthread_mutex_unlock(&thread_input[0].mutexsum);
    }
    return NULL; 
}

pthread_t thread_id1;
pthread_t thread_id2;

void create_debug_thread(void) {
    void *ret;
    printf("Before Thread\n");
    pthread_create(&thread_id1, NULL, debug_rx_start_routine, NULL);
    printf("After thread Thread\n");
}

MD_STATUS  R_UART0_Receive(uint8_t * const rx_buf, uint16_t rx_num) {
    printf("Enter: R_UART0_Receive() locking \n");
    pthread_mutex_lock (&thread_input[0].mutexsum);
    thread_input[0].rx_buf = rx_buf;
    thread_input[0].rx_num = rx_num;
    thread_input[0].present_rx_index = -1;
    pthread_mutex_unlock (&thread_input[0].mutexsum);
    printf("Return: R_UART0_Receive() ; unlocked \n");
    return 0;
}

MD_STATUS R_UART1_Receive(uint8_t * const rx_buf, uint16_t rx_num) {   
    //printf(" >>> R_UART1_Receive() \n");
    pthread_mutex_lock (&thread_input[1].mutexsum);
    thread_input[1].rx_buf = rx_buf;
    thread_input[1].rx_num = rx_num;
    thread_input[1].present_rx_index = -1;
    pthread_mutex_unlock (&thread_input[1].mutexsum);
    //printf(" <<< R_UART1_Receive() \n");
}
#endif
void R_UART0_Send_blocking( uint8_t *tx_buf, uint16_t tx_num) {

	g_Uart0TxEnd = 0;
	R_UART0_Send(tx_buf, tx_num );
	while(0 == g_Uart0TxEnd );
}

void UART0_print_string(uint8_t * tx_buf) {
    while(*tx_buf) {
        g_Uart0TxEnd = 0;
        R_UART0_Send(tx_buf, 1);
        while(0 == g_Uart0TxEnd ){
        }
        tx_buf++;
    }
	//print a newline
	g_Uart0TxEnd = 0;
	R_UART0_Send((uint8_t * )"\n", 1 );
	while(0 == g_Uart0TxEnd ) {
    }
}

void UART0_print_hex_string(uint8_t * tx_buf) {
    while(*tx_buf) {
        g_Uart0TxEnd = 0;
        R_UART0_Send(tx_buf, 1);
        while(0 == g_Uart0TxEnd ){
        }
        tx_buf++;
    }

	//print a newline
	g_Uart0TxEnd = 0;
	R_UART0_Send((uint8_t * )"\n", 1 );
	while(0 == g_Uart0TxEnd ) {
    }
}

void R_UART1_Send_blocking(uint8_t * const tx_buf, uint16_t tx_num) {
	g_Uart1TxEnd = 0;
	R_UART1_Send(tx_buf, tx_num );
	while(0 == g_Uart1TxEnd );
}

void my_R_UART0_Receive(uint8_t *  rx_buf, uint16_t rx_num) {
	g_Uart0RxEnd = 0;
	R_UART0_Receive(rx_buf, rx_num);
	while(0 == g_Uart0RxEnd );
}

void R_UART0_Receive_blocking(uint8_t *  rx_buf, uint16_t rx_num)
{
	g_Uart0RxEnd = 0;
	R_UART0_Receive(rx_buf, rx_num);
	while(0 == g_Uart0RxEnd ) {
		R_WDT_Restart();
	}
}

void R_UART0_Receive_non_blocking(uint8_t *  rx_buf, uint16_t rx_num) {
	g_Uart0RxEnd = 0;
	R_UART0_Receive(rx_buf, rx_num);
}

/*
 * return 0 if there is no data
 * return 1 if there is some data
 * */
int has_R_UART0_Receive_data(void)
{
#if MACHINE == RL78   
	if(g_Uart0RxEnd) {
		return 1;//yes it has some data
	}
	return 0;//No
#elif MACHINE == UBUNTU
    //printf(">>> has_R_UART0_Receive_data()\n");
    int ret_val;
    if (-1 != thread_input[0].present_rx_index ) {
        return 1;// yes there is data data
    } 
    return 0; // no data
#else
#error You have to use the Debug mode
#endif
}

void R_UART1_Receive_blocking(uint8_t *  rx_buf, uint16_t rx_num) {
	g_Uart1RxEnd = 0;
	R_UART1_Receive(rx_buf, rx_num);
	while(0 == g_Uart1RxEnd );
}

void R_UART1_Receive_non_blocking(uint8_t *rx_buf, uint16_t rx_num) {
	g_Uart1RxEnd = 0;
	R_UART1_Receive(rx_buf, rx_num);
}

int R_UART1_Receive_is_full(void)
{
	if(g_Uart1RxEnd) {
		return 1; // yes it is full of requested bytes
	}
	return 0; // Not yet full
}

