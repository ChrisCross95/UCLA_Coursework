#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>


 
const int B  = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k

sig_atomic_t volatile run_flag = 1; 
unsigned int period = 1; 
char scale          = 'F'; 
int logfd; 
bool log_flag = false; 
bool report_flag = true; 

#define BUF_SIZE 256


// declare temperature sensory as mraa_aio_context variable (AIO)
mraa_aio_context temp_sensor;

// delcare button as a mraa_gpio_context variable (GPIO)
mraa_gpio_context button;

void 
shutdown() {

	mraa_aio_close(temp_sensor);
	mraa_gpio_close(button);

	//if (log_flag) { close(logfd); }

	time_t rawtime;
	struct tm *info;
	time( &rawtime );
	info = localtime( &rawtime );
        printf("%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);

	if (log_flag) {
		dprintf(logfd,"%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);
	}

}


void 
do_when_interrupted() 
{
	run_flag = 0;
}


char* use_msg = "Correct usage: lab1b-server [--period][--scale][--log]\n";
void
get_opts(int argc, char **argv) {
    int returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            {"period",    required_argument, 0, 'p'},
            {"scale",     required_argument, 0, 's'},
            {"log",       required_argument, 0, 'l'},
            {0, 0, 0, 0}
        };
        
        int option_index = 0;
        returned_option = getopt_long(argc, argv, "p:s:l:", long_options, &option_index);
        
        /* End of options */
        if (returned_option == -1) { break; }
        
        switch (returned_option) {
            case 'p':
                period = strtol(optarg, NULL, 0);
		if (period < 1) {
			fprintf(stderr, "sampling interval must be postive integer\n");
			exit(1);
		}
                break;
            case 's':
                scale = optarg[0];
		if ( (scale != 'F') && (scale != 'C') ) {
			fprintf(stderr, "Scale must be fahrenheit or celsius\n");
			exit(1);
		}
                break;
            case 'l':
		logfd = open(optarg, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
		if (logfd < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		log_flag = true; 		
                break;
            default: // for unrecognized options passed to main
                fprintf(stderr, "%s", use_msg);
                exit(1);
                break;
        }
    }

    return; 
}


/* SCALE=F
This command should cause all subsequent reports to be generated in degrees Fahrenheit.
SCALE=C
This command should cause all subsequent reports to be generated in degrees Centegrade
PERIOD=seconds
This command should change the number of seconds between reporting intervals. It is acceptable if this command does not take effect until after the next report.
STOP
This command should cause the program to stop generating reports, but continue processing input commands. If the program is not generating reports, merely log receipt of the command.
START
This command should cause the program to, if stopped, resume generating reports. If the program is not stopped, merely log receipt of the command.
LOG line of text
This command requires no action beyond logging its receipt (the entire command, including the LOG). In project 4C, the server may send such commands to help you debug problems with your reports.
OFF
This command should, like pressing the button, output (and log) a time-stamped SHUTDOWN message and exit.
*/

void
exec_cmd(char *cmd_str) {

	if (log_flag) {
		dprintf(logfd,"%s\n", cmd_str);
        }

	char *ptr = NULL;
	if ( ! strcmp(cmd_str, "OFF") ) {
		exit(0); 
	}

        else if ( ! strcmp(cmd_str, "START") ) {
                report_flag = true; 
        }

        else if ( ! strcmp(cmd_str, "STOP") ) {
                report_flag = false; 
        }

	else if ( ! strncmp(cmd_str, "LOG", 3) ) {
                // This command requires no action beyond logging its receipt (the entire command, including the LOG)
        }

	else if ( ! strncmp(cmd_str, "SCALE=", 6) ) {
		ptr = strstr(cmd_str, "SCALE=");
		ptr += 6;
		scale = *ptr; 
		if ( (scale != 'F') && (scale != 'C') ) {
			fprintf(stderr, "Scale must be fahrenheit or celsius\n");
			exit(1);
		}
	}

	else if ( ! strncmp(cmd_str, "PERIOD=", 7) ) {
		ptr = strstr(cmd_str, "PERIOD=");
		ptr += 7;
		char temp_pd[8];
		memset(temp_pd, 0, 8);  
		for (int i = 0; i < 8; i++, ptr++) {
			temp_pd[i] = *ptr;
		}
                period = strtol(temp_pd, NULL, 0);
                if (period < 1) {
                        fprintf(stderr, "sampling interval must be postive integer\n");
                        exit(1);
                }
	}
	else {
		fprintf(stderr, "Unrecognized command\n");
	}

}

void*
sensor_read_thrd() {
	

	while (run_flag) {

                // - - - - - - - - - - - - - Get Reading from Temp Sensory - - - - - - - - - - - - - -
                // read from sensor
                int a = mraa_aio_read(temp_sensor);
                if (a == -1) {
                        fprintf(stderr, "%s\n", strerror(errno));
                        exit(1);
                }

                // convert to fahrenheit
                float R = 1023.0/a-1.0;
                R *= R0;
                float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

		if (scale == 'F') {
			float diff = 32; 
			float ratio =  1.8;
			temp = (ratio*temp) + diff; 
		}

                time_t rawtime;
                struct tm *info;
                time( &rawtime );
                info = localtime( &rawtime );

                if (report_flag) {
                        printf("%02d:%02d:%02d %4.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temp);

                        if (log_flag) {
                                dprintf(logfd,"%02d:%02d:%02d %4.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temp);
                        }
              
			// sleep for PERIOD
			 sleep(period);
		}
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	}

	return NULL; 
}

int
main(int argc, char** argv)
{

	// Get CLO
	get_opts(argc, argv);

	if ( atexit(shutdown) != 0) {
		fprintf(stderr, "cannot set exit function\n");
		exit(1); 
	} 	

	// initalize MRAA pin 1 (A0/A1) for temperature sensory
	temp_sensor = mraa_aio_init(1); 
	if (temp_sensor == NULL) {
        	fprintf(stderr, "Failed to initialize AIO\n");
        	mraa_deinit();
        	exit(1); 
        }

	// initialize MRAA pin 62 (gpio_51) for buzzer.
	button = mraa_gpio_init(60);  
	  if (button == NULL) {
		fprintf(stderr, "Failed to initialize GPIO\n");
        	mraa_deinit();
        	exit(1);
        }
	
 
	// configure button GPIO interface to be an input pin. 
	mraa_gpio_dir(button, MRAA_GPIO_IN);

	// when the button is pressed, call do_when_interrputed 
	mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL); 

	int fd_status;
	fd_status = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	if (fd_status == -1) {
		fprintf(stderr, "Could not set stdin to non-blocking i/o\n");
		exit(1); 
	}


	// Create thread to read from sensor
	int ret_code;
	pthread_t temp_snsr_thrd_id;
	ret_code = pthread_create(&temp_snsr_thrd_id, NULL, sensor_read_thrd, NULL);
	if (ret_code) {
		fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
		exit(1); 
	}

	
	// This small buffer hold a single command string, e.g. STOP. 
	// It is declared outside the while loop since its contents my
	// have to perserve data for a partial read of a command. 
	// Each time a complete command is read into cmd_str (indicated
	// by a newline character) the command is executed, cmd_str is
	// zero-ed and cmd_str_idx is reset to 0. The wdith of cmd_str
	// is length of the largest possible command string. 
	char cmd_str[32];
	int cmd_str_idx  = 0;

	// execute if run_flag is logical true (1).
	// break while loop if run_flag is logical false (0).
	while (run_flag) {

		// - - - - - - - - - - - - - - Get Input From Keyboard - - - - - - - - - - - - - - - -


		char read_buf[BUF_SIZE + 1];
		ssize_t read_len;

		read_len = read(STDIN_FILENO, read_buf, BUF_SIZE); 
		if (read_len == -1) {
			if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
				fprintf(stderr, "read() error: %s\n", strerror(errno));
				exit(1);
			}
		}

		
		if (read_len > 0) {

			for (int i = 0; i < read_len; ++i, ++cmd_str_idx) {
				cmd_str[cmd_str_idx] = read_buf[i];
				
				// We've read a complete command into cmd_str
				if (read_buf[i] == '\n') {
					cmd_str[cmd_str_idx] = '\0'; 
					// fprintf(stderr, "User command: %s\n", cmd_str); 
					exec_cmd(cmd_str); 
					// memset(cmd_str, 0, 32); 
					cmd_str_idx = -1; // This will be incremented to 0.  
				}	

			}
		}

		if (write(STDOUT_FILENO, read_buf, read_len) == -1) {
		//	fprintf(stderr, "write() error: %s\n", strerror(errno));
			// exit(1);
		}	

	}

	// mraa_aio_close(temp_sensor); 
	// mraa_gpio_close(button); 

	exit(0); 
}
