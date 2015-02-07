/*
 * px4_simple_app.c
 *
 *  Created on: Nov 7, 2014
 *      Author: svankoot
 */

#include <poll.h>
#include <uORB/topics/sensor_combined.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>

__EXPORT int my_custom_app_main(int argc, char *argv[]);

int my_custom_app_main(int argc, char *argv[])
{
	printf("Hello Sky!\n");

	//subscribe to a topic and get a file descriptor for it
	int sensor_sub_fd = orb_subscribe(ORB_ID(sensor_combined));

	struct pollfd fds[] = {
			{.fd = sensor_sub_fd, .events = POLLIN },
			/* there could be more file descriptors here, in the form like:
			 * { .fd = other_sub_fd,   .events = POLLIN },
			 */
	};

	int error_counter = 0;

	while (true) {
		//wait for sensor update of 1 file descriptor for 1000ms
		int poll_ret = poll(fds, 1, 1000);

		if (poll_ret == 0) {
			//poll didn't return anything
			printf("[my_custom_app] Got no data within a second\n");
		} else if (poll_ret < 0) {
			//ERROR, return shouldn't be negative
			if (error_counter < 10 || error_counter % 50 == 0) {
				//^make sure you don't spam
				printf("[my_custom_app] ERROR return value from poll(): %d\n",
						poll_ret);
			}
			error_counter++;
		} else {
			if (fds[0].revents & POLLIN) {
				//data for first file descriptor
				struct sensor_combined_s raw;
				//copy sensors raw data to local buffer
				orb_copy(ORB_ID(sensor_combined), sensor_sub_fd, &raw);
				printf("[my_custom_app] Accelerometer:\t%8.4f\t%8.4f\t%8.4f\n",
							(double)raw.accelerometer_m_s2[0],
							(double)raw.accelerometer_m_s2[1],
							(double)raw.accelerometer_m_s2[2]);
			}
		}
	}
	return 0;
}



