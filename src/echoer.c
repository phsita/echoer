/******************************************************************************
 * ECHOER CLIENT PROGRAM
 * TODO- WRITE THIS PROLOGUE
 *****************************************************************************/

/* Standard includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/* This header is required for closing file_descriptor */
#include<unistd.h>

/* This header is required for command options parsing */
#include<getopt.h>

/* This header is required for creating & working with socket */
#include<sys/socket.h>

/* This header is required for validating ipv4 address */
#include<arpa/inet.h>


/*
Ignore these declarations, it will go away soon!
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
*/

#define  VERBOSE_BUFF   2048                // Buffer of verbose statements
#define  ECHO_BUFF_SIZE 1024                // Buffer of messages sent from cli
#define  SOCK_ADDR      struct sockaddr     // Alias for `struct sockaddr'


/******************************************************************************
 * VARIABLES
 *****************************************************************************/

/* Flags set by cmd args ‘-v, --verbose’, ‘-V, --version’, ‘-h, --help’ */
static int flag_verbose = 0, flag_version = 0, flag_help = 0;

/* char arr 'verb_msg_buff' for printing messages */
static char verb_msg_buff[VERBOSE_BUFF] = {'\0'};

/* Program name & Program version */
char *prog_name = NULL, *prog_version = "0.1";

char *server_ip_addr        = NULL;         /* Server listening address */
int  server_port            = 0;            /* Server listening port    */

/*
// Flag set by ‘system's little-endianness (host-order)’
int flag_host_order         = 0;
*/


/******************************************************************************
 * FUNCTIONS
 *****************************************************************************/


/*
 * print_verbose(): prints verbose messages
 */
void print_verbose(char *str) {
    if ( flag_verbose ){
        fprintf( stdout, "[INFO] %s\n",str);
    }
}

/*
 * get_input(): get input from user
 */
char *get_input(int max_len){

    char  ch;
    char *ret_str = NULL, *mem = NULL;
    int   ctr = 0;

    while ( ctr == 0 ) {        /* Outer while loop start     */

        do {                    /* Inner do-while loop start  */
            ch=getchar();
            if( ch == '\n' ) {
                break;
            }
            ++ctr;
            mem = (char *)realloc( ret_str, ctr * sizeof(char) );
            bzero(verb_msg_buff, sizeof(verb_msg_buff));
            sprintf(
                     verb_msg_buff, "Alloc/Re-alloc memory to Snd msg, "
                     "ret str size = %d, ctr = %d, ret_str = %p, mem = %p",
                     sizeof(ret_str), ctr, ret_str, mem
                    );
            print_verbose(verb_msg_buff);
            ret_str = (char *)mem;
            /* if( ret_str == NULL ) { */
            /*     ret_str = (char *) malloc( sizeof(char) * 1 ); */
            /*     sprintf(verb_msg_buff, "Alloc memory to Snd msg (size = %d bytes)", sizeof(ret_str)); */
            /* } else { */
            /*     ret_str = (char *) realloc(ret_str, sizeof(char) * ctr ); */
            /*     sprintf(verb_msg_buff, "Re-Alloc memory to Snd msg (size = %d bytes)", sizeof(ret_str)); */
            /* } */
            /* print_verbose(verb_msg_buff); */
            *(ret_str + ( ctr - 1) ) = ch;
            /* ++ctr; */
        } while( 1 );           /* Inner do-while loop end    */

        if ( ctr == 0 ) {
            fprintf( stdout,
                     "\n\nWARNING! No input given, "
                     "please enter message:\n"
                    );
            continue;
        }
    }                           /* Outer while loop end       */

    return ret_str;
}


/*
 * set_prog_name(): set program name from sys_arg[0]
 */
char *set_prog_name( char *arg0 ) {
    int i, j;       /* loop index counters */
    int arg0_len = strlen(arg0), ptr=-1;

    for ( i=arg0_len ; i >= 0 ; --i) {
        if ( *(arg0+i) == '/' ) {
            ptr=i;
            break;
        }
    }

    if ( ptr != -1 ){
        prog_name = (char *) malloc( sizeof(char) * (arg0_len - i) );
        for ( i = ptr+1, j=0 ; i < arg0_len ; ++i, ++j)
            *(prog_name+j) = *(arg0+i);
    }
    else {
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
        "  %s [-h] [-v] [-V] <server_address> <server_port>\n\n",
        prog_name
      );
}


/*
 * print_usage(): prints program usage and cmd args information
 */
void print_help() {
    fprintf( stdout,
        "Args:\n"
        "  <server_address> HOST_ADDRESS\n"
        "        IPv4 Address of Echoerd server\n"
        "  <server_port>    PORT\n"
        "        TCP Port number of Echoerd server\n"

        "Options:\n"
        "  -h, --help\n"
        "        Print program help\n"
        "  -v, --verbose\n"
        "        Print extra information during program execution\n"
        "  -V, --version\n"
        "        Print version and legal information\n"
        "\n\n"
        "This utility is part of ECHOER project.\n"
        "ECHOER project is made for the purpose of demonstrating\n"
        "client-server communication over a computer network.\n\n"

        "This (ECHOER) is the client program of the ECHOER package\n"
        "Client connects to the server at a specified server-address.\n"
        "Upon successful connection, client can send messages to the\n"
        "server. To which, server replies back with same message\n\n"

        "ECHOERD runs in server-mode and listens on a socket address\n"
        "It accepts connections from clients on a user-specified\n"
        "network address.\n"
        "Upon a successful connection, ECHOERD accepts messages from\n"
        "the ECHOER client.\n"
        "For every message received from client, server sends back\n"
        "the same message as a response to the client (hence the\n"
        "name 'ECHOER').\n\n"

      ); /* fprint( stdout, "...msg" ); end */
}


/*
 * print_version(): prints program version information
 */
void print_version() {
    fprintf( stdout,
        /* MIT LICENSE TEXT BODY <https://opensource.org/licenses/MIT> */
        "Echoer  %s\n"

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
 * validate_port(): validate port values and return
 */
int validate_port( char *port, int *mem_address ) {

    int i, port_len = strlen( port );
    if ( port_len > 5 ) {
        fprintf( stderr, "Error! TCP Port '%s' is invalid\n", port );
        exit(1);
    }
    else {
        for ( i=0; i < port_len ; ++i ){
            if ( (*(port+i) < 48) || (*(port+i) > 58) ) {
                fprintf(stderr, "Error! TCP Port '%s' is invalid\n", port);
                exit(1);
            }
        }
    }

    *mem_address = atoi(port);

    if ( *mem_address == 0 || *mem_address >= 65536 ) {
        fprintf( stderr, "Error! Port '%s' is invalid\n", port );
        exit(1);
    }

    return 0;
}


/*
 * validate_address(): validate ipv4 address and return
 */
int validate_address( char *ip_addr, char *mem_addr ) {

    unsigned char *tmp_dst = (unsigned char *) malloc(sizeof(struct in6_addr));

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


    int c;                  /* Store getopt_long return code */

    while (1) {
        static struct option long_options[] = {
            { "help",     no_argument,          NULL,      'h' },
            { "version",  no_argument,          NULL,      'V' },
            { "verbose",  no_argument,          NULL,      'v' },
            { 0, 0, 0, 0 }
        };

        int opt_ind = 0;    /* getopt_long returns 'option index' */

        c = getopt_long( sys_argc, sys_argv, "hvV",
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

            case '?':       /* all other unknown values */
                /* getopt_long already printed an error message. */
                bzero(verb_msg_buff, sizeof(verb_msg_buff) );
                sprintf( verb_msg_buff, "C = %c", c );
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

    /* Check for expected arguments */
    if ( (sys_argc - optind) != 2  ) {
        if ( (sys_argc - optind) < 2)
            fprintf( stderr, "ERROR! Too few arguments specified." );
        else
            fprintf( stderr, "ERROR! Too many arguments specified." );
        print_usage();
        exit(1);

    } else {
        /* Process remaining command line arguments (not options). */
        int cnt=1;

        while ( optind < sys_argc ) {

            if ( cnt == 1) {

                bzero(verb_msg_buff, sizeof(verb_msg_buff) );
                sprintf( verb_msg_buff, "Address arg:\t\"%s\"", sys_argv[optind]);
                print_verbose(verb_msg_buff);

                server_ip_addr = (char *) malloc(
                    sizeof(char) *
                    (strlen(sys_argv[optind]))
                  );
                validate_address( sys_argv[optind], server_ip_addr );
            } else if (cnt == 2) {

                bzero(verb_msg_buff, sizeof(verb_msg_buff) );
                sprintf( verb_msg_buff, "Port arg:\t\"%s\"", sys_argv[optind]);
                print_verbose(verb_msg_buff);

                validate_port( sys_argv[optind], &server_port );
            }
            ++cnt;
            ++optind;
        }
    }
}

/*
 * Communicate with server
 */
void echo_it(int connfd) {

    // char snd_msg[ECHO_BUFF_SIZE]; // snd_msg[ECHO_BUFF_SIZE];
    char rec_msg[ECHO_BUFF_SIZE], snd_msg[ECHO_BUFF_SIZE]; // *snd_msg = NULL;
    ssize_t sent_bytes, read_bytes;
    int snd_msg_len, rec_msg_len;
    // ssize_t rec_msg_size;
    // size_t rec_msg_len;

    /* conditional client-server echoer loop */
    for (;;) {
        /* Initialize with zero */
        snd_msg_len = 0;
        rec_msg_len = 0;
        sent_bytes  = 0;
        read_bytes  = 0;

        bzero( snd_msg, ECHO_BUFF_SIZE );
        bzero( rec_msg, ECHO_BUFF_SIZE );

        printf( "Enter message to send to server:\n" );
        scanf("%s", snd_msg );
        /* snd_msg = get_input(ECHO_BUFF_SIZE); */
        snd_msg_len = strlen(snd_msg);

        /* verbose print snd_msg */
        bzero( verb_msg_buff, VERBOSE_BUFF );
        sprintf( verb_msg_buff, "CLIENT MESSAGE = \"%s\" [%d bytes]",
                 snd_msg, snd_msg_len );
        print_verbose( verb_msg_buff );

        /* Send snd_msg to server */
        sent_bytes = send( connfd, snd_msg, snd_msg_len, 0 );

        /* if retval == -1 , send has errors */
        if ( sent_bytes == -1  ) {
            /* free(snd_msg);        /\* done with sent-msg *\/ */
            /* send failed, break loop and get out of function */
            perror( "send()" );
            break;
        } else {
            /* verbose print snd_msg */
            bzero( verb_msg_buff, VERBOSE_BUFF );
            sprintf( verb_msg_buff,
                     "%d bytes message sent to server!",
                     sent_bytes );
            print_verbose( verb_msg_buff );
        }

         /* if snd_msg = "quit", terminate client program */
        if ( strncmp( snd_msg, "quit" , 4 ) == 0 ) {
            /* free(snd_msg);        /\* done with sent-msg *\/ */
            print_verbose( "Quit message issued to server. Terminating.\n");
            break;
        }

        /* free(snd_msg);        /\* done with sent-msg *\/ */

        /* read client msg from socket, store in rec_msg */
        read_bytes = recv( connfd, rec_msg, ECHO_BUFF_SIZE, 0 /*no flags*/ );

        /* if retval == -1 , send has errors */
        if ( read_bytes == -1  ) {
            /* receive failed, break loop and get out of function */
            perror( "Error: Recv message: " );
            break;
        } else if ( read_bytes == 0 ){
            fprintf( stdout,
                     "Received bytes = %d\n",
                     read_bytes
                    );
        } else if ( rec_msg == NULL ){
            fprintf( stderr,
                     "Received msg buffer * = NULL. Terminating program.\n" );
        } else {
            /* verbose print snd_msg */
            bzero( verb_msg_buff, VERBOSE_BUFF );
            sprintf( verb_msg_buff, "Received %d bytes from server!",
                     read_bytes );
            print_verbose( verb_msg_buff );
        }

        /* print server's msg */
        fprintf( stdout, "[SERVER REPLY] \"%s\" [%d bytes]\n",
                 rec_msg, read_bytes );

        /* if rec_msg = "quit", terminate client program */
        if ( strncmp( rec_msg, "quit" , 4 ) == 0 ) {
            fprintf( stdout,
                "Quit message issued by server. Terminating...\n"
              );
            break;
        }
    }
}


/*
 * MAIN FUNCTION WITH ARGS
 */
int main ( int argc, char **argv ) {

    int sock_fd;            /* var to store socket file descriptor    */
    int conn_rc;            /* var to store return code of connect()  */

    /* server_addr structure for socket creation  */
    struct sockaddr_in server_addr;


    set_prog_name(argv[0]); /* set prog_name from command line arg[0] */

    /* validate and set command line args */
    validate_input( argc, argv );

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);      /* Create socket  */

    /* Check if socket is created successfully by socket() */
    if ( sock_fd == -1 ) {
        fprintf( stdout, "Failed to create socket\n");
        perror( "Error: " );
        exit(1);
    } else {
        fprintf( stdout, "Socket created successfully!\n");
    }

    /* Zero variable 'server_addr' */
    bzero( &server_addr, sizeof(server_addr) );

    /* Set values of server_addr struct fields */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip_addr);
    server_addr.sin_port = htons(server_port);

    /* Connect to the server, store return code at conn_rc */
    conn_rc = connect( sock_fd, (SOCK_ADDR*) &server_addr, sizeof(server_addr));

    if ( conn_rc != 0 ){
        /* connection failed! */
        fprintf( stderr, "Failed to connect to the server!\n" );
        perror( "Error: " );
        exit(1);
    } else {
        /* connection successful! */
        fprintf( stdout, "Connected with server!\n" );
        /* print_verbose( "Sleep #1; 5 s3c0nds!" ); */
        /* sleep( 5 ); */
    }

    /* Communicate with server */
    echo_it(sock_fd);

    /* Socket close when communication is finished */
    close(sock_fd);

    /* Free 'server_ip_addr' object */
    free(server_ip_addr);

    /* Free 'prog_name' object */
    free(prog_name);

    return 0;
}

/******************************************************************************
 * REFERENCES
 *
 * http://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
 * https://www.cs.cmu.edu
 * https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
 * https://www.mkssoftware.com/docs/man3/listen.3.asp
 * https://pubs.opengroup.org/onlinepubs/009695399/functions/accept.html
 * https://www2.cs.arizona.edu/~mccann/cstyle.html
 *
 *****************************************************************************/


