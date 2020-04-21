/******************************************************************************
 * ECHOER SERVER
 * ver 0.1
 * this version uses select() call for communicating with multiple clients
 * flow -

create socket
bind socket
setsockopt
listen
fd_set
loop
    select
    if ( fd_isset )
        new_client_fd = accept()
        if ( new_client_fd > fdmax ) { fdmax = new_client_fd }
        <!---    recv <--> send    ---!>
        if ( msg == close | error )
            close
            FD_CLR
        else
            send
close
---------
main
driver -- creates a listener
worker/server -- looks for all clients connections
talk -- recv + send

 *****************************************************************************/

/* Standard includes */
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>

/* This header is required for command options parsing */
#include<getopt.h>

/* This header is required for creating & working with socket */
#include<sys/socket.h>

/* This header is required for validating ipv4 address */
#include<arpa/inet.h>


#define  VERBOSE_BUFF   2048                // Buffer of verbose statements
#define  ECHO_BUFF_SIZE 255                 // Buffer of messages sent from cli
#define  SOCK_ADDR      struct sockaddr     // Alias for `struct sockaddr'


/******************************************************************************
 * VARIABLES
 *****************************************************************************/

/* Flag to terminate program */
volatile bool flag_terminate = false;

/* Flags set by cmd args -v, --verbose, -V, --version, -h, --help */
static int flag_verbose = 0, flag_version = 0, flag_help = 0;

/* Char array to print verbose messages */
static char verb_msg_buff[VERBOSE_BUFF] = {'\0'};

/* Program name & Program version */
char *prog_name = NULL, *prog_version = "0.1";

char *server_ip_addr        = NULL;         // Server listening address
int   server_port           = 0;            // Server listening port
char *client_ip_addr        = NULL;         // Client ip address


/******************************************************************************
 * FUNCTIONS
 *****************************************************************************/


/*
 * set_prog_name(): set program name from sys_arg[0]
 */
char *set_prog_name( char *arg0 ) {
    int i, j;  /* loop index counters */
    int arg0_len = strlen(arg0), ptr=-1;

    for( i=arg0_len ; i >= 0 ; --i ) {
        /* use '/' as separator for basename */
        if ( *(arg0+i) == '/' ) {
            ptr=i;
            break;
        }
    }

    if ( ptr != -1 ) {
        prog_name = (char *) malloc( sizeof(char) * (arg0_len - i) );
        for ( i = ptr+1, j=0 ; i < arg0_len ; ++i, ++j)
            *(prog_name+j) = *(arg0+i);
    } else {
        prog_name = (char *) malloc( sizeof(char) * arg0_len );
        strcpy( prog_name , arg0 );
    }

}


/*
 * print_usage(): prints program usage information
 */
void print_usage() {
    fprintf( stdout,
        "\nUsage:\n"
        "  %s [-a host_address] [-p port_number] [-h] [-v] [-V]\n\n",
        prog_name
      );
}


/*
 * print_usage(): prints program usage and cmd args information
 */
void print_help() {
    fprintf( stdout,
        "Options:\n"
        "  -a, --address HOST_ADDRESS\n"
        "        IPv4 Address to use for server\n"
        "        Default address: \"0.0.0.0\"\n"
        "  -p, --port    PORT_NUM\n"
        "        TCP Port number to use for server\n"
        "        Default port: \"1970\"\n"
        "  -h, --help\n"
        "        Print program help and options\n"
        "  -v, --verbose\n"
        "        Print extra information during program execution\n"
        "  -V, --version\n"
        "        Print version and legal information\n"
        "\n\n"

        "This utility is part of ECHOER project.\n"
        "ECHOER project is made for the purpose of demonstrating\n"
        "client-server communication over a computer network.\n\n"

        "This (ECHOERD) is the server program of the ECHOER package\n"
        "It runs in server-mode and listens on a socket address\n"
        "It accepts connections from clients on a user-specified\n"
        "network address.\n"
        "Upon a successful connection, server accepts messages from\n"
        "the ECHOER CLIENT.\n"
        "For every message received from CLIENT, server sends back\n"
        "the same message as a response to the client (hence the\n"
        "title 'ECHOER').\n\n"

        "ECHOERD is the client program of the ECHOER package.\n"
        "Client connects to the server at a specified server-address.\n"
        "Upon successful connection, client can send messages to the\n"
        "server. To which, server replies back with same message\n\n"

      ); /* fprint( stdout, "...msg" ); end */
}


/*
 * print_version(): prints program version information
 */
void print_version() {
    fprintf(
        stdout,

        /* MIT LICENSE TEXT BODY <https://opensource.org/licenses/MIT> */
        "Echoerd  %s\n"

        "MIT License <https://opensource.org/licenses/MIT>\n\n"

        "Copyright (c) 2020 Atish Pandey, April 2020\n\n"

        "Permission is hereby granted, free of charge, to any\n"
        "person obtaining a copy of this software and associated\n"
        "documentation files (the \"Software\"), to deal in the\n"
        "Software without restriction, including without\n"
        "limitation the rights to use, copy, modify, merge,\n"
        "publish, distribute, sublicense, and/or sell copies of\n"
        "the Software, and to permit persons to whom the\n"
        "Software is furnished to do so, subject to the\n"
        "following conditions:\n\n"

        "The above copyright notice and this permission notice\n"
        "shall be included in all copies or substantial\n"
        "portions of the Software.\n\n"

        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF\n"
        "ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED\n"
        "TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\n"
        "PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT\n"
        "SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
        "CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION\n"
        "OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR\n"
        "IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
        "DEALINGS IN THE SOFTWARE.\n\n",

        prog_version

      ); /* fprint( stdout, "...msg" ); end */
}


/*
 * print_verbose(): prints verbose messages
 */
void print_verbose(char *str) {
    if ( flag_verbose ){
        fprintf( stdout, "[INFO] %s\n", str);
    }
}


/*
 * validate_port(): validate port values and return
 */
int validate_port( char *port ) {

    int port_num;

    if ( ( strlen( port ) == 1 ) && ( *port == '0' ) ) {
        port_num = 0;
    } else if ( ( (port_num = atoi(port))  <= 0 ) || ( port_num > 65535 ) )  {
        fprintf( stderr, "Error! TCP Port '%s' is invalid\n", port ); 
        exit(1);
    }

    return port_num;
}


/*
 * validate_address(): validate ipv4 address and return
 */
int validate_address( char *ip_addr, char *mem_addr ) {

    unsigned char *tmp_dst = (unsigned char *) malloc(sizeof(struct in_addr));

    if ( inet_pton(AF_INET, ip_addr, tmp_dst) == 1 )
        strncpy( mem_addr, ip_addr, ( strlen( ip_addr ) + 1 ) );
    else {
        fprintf( stderr, "IPv4 address '%s' invalid!\n", ip_addr );
        exit(1);
    }

    free(tmp_dst);
    return 0;
}


/*
 * validate_input(): validate cmdline input
 */
void validate_input(int sys_argc, char **sys_argv) {

    int c;

    /* Flag set by -p, --port and -a, --address */
    char flag_port='0', flag_addr='0';

    while (1) {
        static struct option long_options[] = {
            { "address",  required_argument,    0,         'a' },
            { "port",     required_argument,    0,         'p' },
            { "help",     no_argument,          NULL,      'h' },
            { "version",  no_argument,          NULL,      'V' },
            { "verbose",  no_argument,          NULL,      'v' },
            { 0, 0, 0, 0 }
        };

        int opt_ind = 0;    /* getopt_long returns 'option index' */

        c = getopt_long( sys_argc, sys_argv, "a:p:hvV",
                long_options, &opt_ind );

        /* Detect the end of the options */
        if ( c == -1 )
            break;          /* end of options */

        switch (c) {

            case 0:
                /* If this option set a flag, do nothing else now. */
                if ( long_options[opt_ind].flag != 0)
                    exit(1);
                fprintf( stdout, "option %s", long_options[opt_ind].name);
                if ( optarg )
                    fprintf( stdout, " with arg %s", optarg );
                fprintf( stdout, "\n" );
                free(prog_name);
                exit(1);

            case 'h':       /* -h | --help    */
                print_usage();
                print_help();
                free(prog_name);
                exit(0);

            case 'v':       /* -v | --version */
                flag_verbose=1;
                break;

            case 'V':       /* -V | --version */
                print_version();
                free(prog_name);
                exit(0);

            case 'a':       /* -a | --address */

                if ( flag_addr == '1' ) {
                    fprintf( stderr,
                        "Error! Multiple address values provided.\n"
                        "Refer usage information (-h, --help)\n",
                        server_ip_addr
                      );
                    exit(1);
                }

                server_ip_addr = (char *) malloc(
                                              sizeof(char) *
                                              (strlen(optarg) + 1)
                                            );
                validate_address( optarg, server_ip_addr );
                flag_addr = '1';
                break;

            case 'p':       /* -p | --port    */

                if ( flag_port == '1' ) {
                    fprintf( stderr,
                        "Error! Multiple port values provided.\n"
                        "Refer usage information (-h, --help)\n"
                      );
                    exit(1);
                }

                server_port = validate_port( optarg );
                flag_port = '1';
                break;

            case '?':       /* all other unknown values */
                /* getopt_long already printed an error message. */
                snprintf( verb_msg_buff, sizeof(verb_msg_buff), "C = %c", c );
                print_verbose( verb_msg_buff );
                print_usage();
                free(prog_name);
                exit(1);

            default:
                /* do nothing */
                printf("DEFAULT CASE, c = %d (%c)\n", c, c);
                print_usage();
                free(prog_name);
                exit(1);
        }
    }

    /* Check for extra arguments */
    if ( (sys_argc - optind) != 0  ) {
        fprintf( stderr, "ERROR! Too many arguments specified." );
        print_usage();
        exit(1);
    }

    /* Validate port value for definition */
    if (flag_port == '0') {
        server_port = 1970;
        snprintf( verb_msg_buff, VERBOSE_BUFF,
            "Server listening port not specified, "
            "using default port: %d", server_port
          );
        print_verbose( verb_msg_buff );
    } else {
        snprintf( verb_msg_buff, VERBOSE_BUFF,
            "Server listening port: %d", server_port
          );
        print_verbose( verb_msg_buff );
    }

    /* Validate address value for definition */
    if (flag_addr == '0') {
        server_ip_addr = (char *) realloc( server_ip_addr, sizeof(char) * 8 );
        strcpy( server_ip_addr , "0.0.0.0" );
        snprintf( verb_msg_buff, VERBOSE_BUFF,
            "Server listening address not specified, "
            "using default address: \"%s\"", server_ip_addr
          );
        print_verbose( verb_msg_buff );
    } else {
        snprintf( verb_msg_buff, VERBOSE_BUFF,
            "Server listening address: \"%s\"",
            server_ip_addr
          );
        print_verbose( verb_msg_buff );
    }

}


/*
 * SERVER FUNCTION
 */
void server() {

    int server_fd, new_client_fd, conn_fd, yes = 1, tmp;
    fd_set master, read_fds;
    int fdmax, fds;
    socklen_t addrlen, client_addr_len;

    struct sockaddr_in server_addr, client_addr;

    char buf[256];
    char remoteIP[INET6_ADDRSTRLEN];
    int i,j;            /* iterator variables */
    struct timeval tv;

    /* Register signal and signal handler     */
    /* signal(SIGINT, sig_handler); */

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);


    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    /* Create socket */
    server_fd = socket( AF_INET, SOCK_STREAM, 0 );

    /* Check if socket is created successfully by socket() */
    if ( server_fd == -1 ) {
        fprintf( stdout, "Failed to create a socket\n");
        perror( "socket():" );
        exit(1);
    } else {
        fprintf( stdout, "Socket created successfully!\n");
    }

    /* clear struct server_addr                */
    bzero( &server_addr, sizeof(server_addr) );

    /* bzero( &client_addr, sizeof(client_addr) ); */

    /* Set values of server_addr struct fields */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    /* Set value of server_addr sin_addr member */
    tmp = inet_pton( AF_INET, server_ip_addr, &server_addr.sin_addr.s_addr );

    if ( ! ( tmp > 0 ) ){
        fprintf( stderr, "Address -- %s conversion failed\n", server_ip_addr );
        perror( "inet_pton()" );
        exit(1);
    }

    /* Bind the created socket to given IP and verify */
    if ( bind(server_fd, (SOCK_ADDR*)&server_addr, sizeof(server_addr)) != 0) {
        /* Bind the created socket to given IP and verify */
        fprintf(
            stderr,
            "Failed to bind socket to %s:%d !\n",
            server_ip_addr, server_port
          );
        perror("bind()");
        exit(1);
    } else {
        fprintf( stdout, "Socket bound successfully\n" );
        if( setsockopt(
                        server_fd, SOL_SOCKET,
                        SO_REUSEADDR, &yes, sizeof(yes)
                       ) == -1 ) {
            perror("setsockopt()");
            exit(1);
        }
    }

    /* Listen to socket address addr:port  */
    if ( (listen(server_fd, 5)) != 0 ) {
        fprintf( stderr,
            "Listen on %s:%d failed\n",
            server_ip_addr, server_port
          );
        perror( "listen()" );
        exit(1);
    } else {
        fprintf( stdout,
            "Server listening for connections on %s:%d\n",
            server_ip_addr, server_port
          );
    }

    /* add server-fd to master fd set */
    FD_SET(server_fd, &master);

    /* initial value of fdmax */
    fdmax = server_fd;
    int temp_ctr = 0;
    while ( 1 ) {              /* server loop */
        snprintf( verb_msg_buff, VERBOSE_BUFF,
                  "  while | [%d] start", temp_ctr );
        print_verbose( verb_msg_buff );

        read_fds = master;

        bzero( verb_msg_buff, VERBOSE_BUFF );
        snprintf( verb_msg_buff, VERBOSE_BUFF,
                  "  while | master -> read_fds" );
        print_verbose( verb_msg_buff );

        /* select() */
        fds = select(fdmax+1, &read_fds, NULL, NULL, &tv );

        bzero( verb_msg_buff, VERBOSE_BUFF );
        snprintf( verb_msg_buff, VERBOSE_BUFF,
                  "  while | fds = %d", fds );
        print_verbose( verb_msg_buff );

        
        if ( fds == -1 ) {
            perror("select()");
            exit(1);
        }

        /* loop through existing connections */
        /* for ( i = 0; i <= fdmax ; i++ ) { */
        for ( i = 0; i <= fdmax ; i++ ) {

            bzero( verb_msg_buff, VERBOSE_BUFF );
            /* snprintf( verb_msg_buff, VERBOSE_BUFF, */
            /*           "  while | for | i [%d] <= fdmax [%d]", */
            /*           i, fdmax ); */
            snprintf( verb_msg_buff, VERBOSE_BUFF,
                      "  while | for start i [%d] <= fdmax [%d] > 0",
                      i, fdmax );
            print_verbose( verb_msg_buff );

            /* --fds; */

            /* bzero( verb_msg_buff, VERBOSE_BUFF ); */
            /* snprintf( verb_msg_buff, VERBOSE_BUFF, */
            /*           "  while | for | --fds => %d", */
            /*           i, fdmax ); */
            /* print_verbose( verb_msg_buff ); */

            /* bzero( verb_msg_buff, VERBOSE_BUFF ); */
            /* snprintf( verb_msg_buff, VERBOSE_BUFF, */
            /*           "within for loop | --fds => %d", fds ); */
            /* print_verbose( verb_msg_buff ); */


            if ( FD_ISSET(i, &read_fds) ) {
                /* we got one */

                bzero( verb_msg_buff, VERBOSE_BUFF );
                snprintf( verb_msg_buff, VERBOSE_BUFF,
                          "  while | for | "
                          "if ( FD_ISSET( i [%d], &read_fds ) => true",
                          i );
                print_verbose( verb_msg_buff );

                if ( i == server_fd ) {

                    bzero( verb_msg_buff, VERBOSE_BUFF );
                    snprintf( verb_msg_buff, VERBOSE_BUFF,
                              "  while | for | "
                              "if ( FD_ISSET( i [%d], &read_fds ) | "
                              "if ( i [%d] == server_fd [%d] ) => true",
                              i, i, server_fd );
                    print_verbose( verb_msg_buff );

                    /* new client connecting */
                    /* handle new connections */
                    client_addr_len = sizeof(client_addr);

                    /* Accept the data packet from client and verification */
                    new_client_fd = accept(
                                            server_fd,
                                            (SOCK_ADDR*) &client_addr,
                                            &client_addr_len
                                           );
                    client_ip_addr = (char*) malloc( INET_ADDRSTRLEN );
                    inet_ntop( AF_INET, &client_addr.sin_addr,
                               client_ip_addr, INET_ADDRSTRLEN );

                    if ( new_client_fd < 0 ) {
                        fprintf( stdout, "Server accept failed!\n" );
                        perror( "accept()" );
                        exit(1);
                    } else {
                        FD_SET(new_client_fd, &master); /* add to master set */
                        fprintf(
                                stdout,
                                "Connected to client: %s!\n",
                                client_ip_addr
                                );

                        bzero( verb_msg_buff, VERBOSE_BUFF );
                        snprintf( verb_msg_buff, VERBOSE_BUFF,
                                  "  while | for | "
                                  "if ( FD_ISSET( i [%d], &read_fds ) | "
                                  "if ( i [%d] == server_fd [%d] ) | "
                                  "if ( new_client_fd >= 0 ) | "
                                  "FD_SET( new_client_fd [%d] , &master )",
                                  i, i, server_fd, new_client_fd );
                        print_verbose( verb_msg_buff );


                        bzero( verb_msg_buff, VERBOSE_BUFF );
                        snprintf( verb_msg_buff, VERBOSE_BUFF,
                                  "  while | for | "
                                  "if ( FD_ISSET( i [%d], &read_fds ) | "
                                  "if ( i [%d] == server_fd [%d] ) | "
                                  "if ( new_client_fd >= 0 ) | "
                                  "if ( fdmax [%d] < new_client_fd [%d] ) ?" ,
                                  i, i, server_fd, fdmax, new_client_fd );
                        print_verbose( verb_msg_buff );

                        if ( fdmax < new_client_fd ) {
                            fdmax = new_client_fd;
                            bzero( verb_msg_buff, VERBOSE_BUFF );
                            snprintf( verb_msg_buff, VERBOSE_BUFF,
                                      "  while | for | "
                                      "if ( FD_ISSET( i [%d], &read_fds ) | "
                                      "if ( i [%d] == server_fd [%d] ) | "
                                      "if ( new_client_fd >= 0 ) | "
                                      "if ( fdmax < new_client_fd ) | "
                                      "fdmax = new_client_fd = %d" ,
                                      i, i, server_fd, fdmax );
                            print_verbose( verb_msg_buff );

                        }
                        
                    }

                    /* new client block end */
                } else {
                    /* received data from a client */
                    /* Communicate with client */

                    bzero( verb_msg_buff, VERBOSE_BUFF );
                    snprintf( verb_msg_buff, VERBOSE_BUFF,
                              "  while | for | "
                              "if ( FD_ISSET( i [%d], &read_fds ) | "
                              "if ( i [%d] == server_fd [%d] ) => false",
                              i, i, server_fd );
                    print_verbose( verb_msg_buff );

                    char rec_msg[ECHO_BUFF_SIZE]; // *snd_msg = NULL;
                    ssize_t rec_msg_size;

                    /* clear rec_msg buffer */
                    bzero(rec_msg, ECHO_BUFF_SIZE);

                    /* read client rec_msg from socket, store in rec_msg */
                    rec_msg_size = recv( i, rec_msg, ECHO_BUFF_SIZE, 0 /*no flags*/ );

                    /* If rec_msg_size = -1, there is error in receive */
                    if ( rec_msg_size <= 0 ){

                        bzero( verb_msg_buff, VERBOSE_BUFF );
                        snprintf( verb_msg_buff, VERBOSE_BUFF,
                                  "  while | for | "
                                  "if ( FD_ISSET( i [%d], &read_fds ) | "
                                  "if ( i [%d] == server_fd [%d] ) { .. } else | "
                                  "if ( rec_msg_size [%d] <= 0 ) => true",
                                  i, i, server_fd, rec_msg_size );
                        print_verbose( verb_msg_buff );
                      
                        if ( rec_msg_size == -1 ){
                            perror("recv()");
                            /* exit(1); */
                        } else if ( rec_msg_size == 0 ) {
                            fprintf( stdout,
                                     "Client closed connection!\n"
                                     );
                            /* break; */
                        }
                        close(i);
                        FD_CLR(i, &master);
                    } else {

                        bzero( verb_msg_buff, VERBOSE_BUFF );
                        snprintf( verb_msg_buff, VERBOSE_BUFF,
                                  "  while | for | "
                                  "if ( FD_ISSET( i [%d], &read_fds ) | "
                                  "if ( i [%d] == server_fd [%d] ) { .. } else | "
                                  "if ( rec_msg_size [%d] <= 0 ) => false",
                                  i, i, server_fd, rec_msg_size );
                        print_verbose( verb_msg_buff );

                        if ( rec_msg == NULL ) {
                            fprintf( stderr,
                                     "Received NULL."
                                     "Closing this connection\n"
                                     );
                            /* clear current fd from master set */
                            close(i);
                            FD_CLR(i, &master);
                        } else {
                            /* verbose print snd_msg */
                            print_verbose( "Received message from client!" );

                            bzero( verb_msg_buff, VERBOSE_BUFF );
                            snprintf( verb_msg_buff, VERBOSE_BUFF,
                                      "  while | for | "
                                      "if ( FD_ISSET( i [%d], &read_fds ) | "
                                      "if ( i [%d] == server_fd [%d] ) { .. } else | "
                                      "if ( rec_msg_size [%d] <= 0 ) { .. } else | "
                                      "if ( rec_msg_size [%d] == NULL ) => false",
                                      i, i, server_fd, rec_msg_size, rec_msg_size );
                            print_verbose( verb_msg_buff );


                            /* print client rec_msg */
                            fprintf(
                                    stdout,
                                    "[CLIENT MESSAGE] \"%s\" [%d]\n",
                                    rec_msg, rec_msg_size
                                    );

                            if ( strncmp( rec_msg, "quit" , 4 ) == 0 ) {
                                fprintf( stdout,
                                         "Quit message issued by client."
                                         "Terminating...\n"
                                         );
                                close(i);
                                FD_CLR(i, &master);

                                bzero( verb_msg_buff, VERBOSE_BUFF );
                                snprintf( verb_msg_buff, VERBOSE_BUFF,
                                          "  while | for | "
                                          "if ( FD_ISSET( i [%d], &read_fds ) | "
                                          "if ( i [%d] == server_fd [%d] ) { .. } else | "
                                          "if ( rec_msg_size [%d] <= 0 ) { .. } else | "
                                          "if ( rec_msg == \"quit\" ) => FD_CLR(i [%d], &master);",
                                          i, i, server_fd, rec_msg_size, i );
                                print_verbose( verb_msg_buff );

                            }

                            /* clear verbose message buffer */
                            bzero( verb_msg_buff, VERBOSE_BUFF );
                            snprintf( verb_msg_buff, sizeof(verb_msg_buff),
                                      "SERVER RESPONSE = \"%s\"", rec_msg );
                            print_verbose( verb_msg_buff );

                            /* Echo received msg back to client */
                            int mmm = send( i, rec_msg, rec_msg_size, 0 );

                            bzero( verb_msg_buff, VERBOSE_BUFF );
                            snprintf( verb_msg_buff, VERBOSE_BUFF,
                                      "  while | for | "
                                      "if ( FD_ISSET( i [%d], &read_fds ) | "
                                      "if ( i [%d] == server_fd [%d] ) { .. } else | "
                                      "if ( rec_msg_size [%d] <= 0 ) { .. } else | "
                                      "send ( i [%d] , rec_msg [%p], "
                                      "rec_msg_size [%d] ) ; ret_code = %d",
                                      i, i, server_fd, rec_msg_size, i, rec_msg, rec_msg_size, mmm );
                            print_verbose( verb_msg_buff );

                        } /* end of conditional block -- recv == NULL or msg */
                    } /* end of conditional block -- recv == 0 or error */
                } /* end of conditional block -- read_fds */

            } else { /* end of conditional block -- FD_ISSET */

                bzero( verb_msg_buff, VERBOSE_BUFF );
                snprintf( verb_msg_buff, VERBOSE_BUFF,
                          "  while | for | "
                          "if ( FD_ISSET( i [%d], &read_fds ) => false",
                          i );
                print_verbose( verb_msg_buff );

            }  /* end of conditional block -- FD_ISSET */

            bzero( verb_msg_buff, VERBOSE_BUFF );
            snprintf( verb_msg_buff, VERBOSE_BUFF,
                      "  while | for end i = %d",
                      i );
            print_verbose( verb_msg_buff );

        } /* end of for loop -- existing connections */
        ++temp_ctr;

        bzero( verb_msg_buff, VERBOSE_BUFF );
        snprintf( verb_msg_buff, VERBOSE_BUFF,
                  "  while | [%d] end", temp_ctr );
        print_verbose( verb_msg_buff );
        /* sleep(2); */
    } /* end of while( flag_terminate ) */
    
    /* Socket close when communication has completed */
    close(server_fd);

    /* Free 'server_ip_addr' object */
    free(server_ip_addr);

}


/*
 * MAIN FUNCTION WITH ARGS
 */
int main ( int argc, char **argv ) {

    /* Register signal and signal handler     */
    // signal(SIGINT, sig_handler);
 
    /* set prog_name from command line arg[0] */
    set_prog_name( argv[0] );

    /* validate and set command line args     */
    validate_input( argc, argv );

    /* start server procedure */
    server();

    /* free prog_name memory */
    free(prog_name);

    return 0;
}

/******************************************************************************
 * REFERENCES
 *
 * https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
 * https://www.cs.cmu.edu
 * https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
 * https://www.mkssoftware.com/docs/man3/listen.3.asp
 * https://pubs.opengroup.org/onlinepubs/009695399/functions/accept.html
 * https://www2.cs.arizona.edu/~mccann/cstyle.html
 *
 *****************************************************************************/

