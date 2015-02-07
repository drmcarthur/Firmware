/*
 * io_test.c
 *
 *  Created on: Nov 10, 2014
 *      Author: svankoot
 */
#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <uORB/uORB.h>
#include <uORB/topics/actuator_armed.h>
#include <uORB/topics/manual_control_setpoint.h>
#include <uORB/topics/test_motor.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/topics/satellite_info.h>
#include <drivers/drv_hrt.h>

#define IS_ECHO 0

__EXPORT int io_test_main(int argc, char *argv[]);

int io_test_main(int argc, char *argv[])
{
	// Advertise/Publish to gps position topic
	struct vehicle_gps_position_s gps_s;
	memset(&gps_s, 0, sizeof(gps_s));
	int gps_pub_fd = 0;

	// Advertise/Publish to satellite info topic
	struct satellite_info_s satellite_s;
	memset(&satellite_s, 0, sizeof(satellite_s));
	int satellite_pub_fd = 0;

	// Variables to store GPS coordinates
	long int lat;
	long int lon;
	long int alt;
	char id;
	char buffer[80];

	// Main Loop (Read from serial 5 and publish GPS data)
	while (1)
	{
		// Read from serial 5 port (get Vicon GPS info through XBee)
		gets(buffer);

		// ************* Check for initialization message: 'BEGIN' *************
		if(strcmp(buffer,"BEGIN") == 0)
		{
			//printf("%s\n",buffer);  // Establish 1st communication
		}
		else
		{
			// TODO: check for (id == this.id) (to ignore other vehicles' messages)
			sscanf(buffer,"%c,%ld,%ld,%ld",&id,&lat,&lon,&alt); // Read gps coordinates

			// DEBUG - Verify that type casting to int32_t doesn't corrupt data
			//int32_t test1 = (int32_t) lat;
			//int32_t test2 = (int32_t) lon;
			//int32_t test3 = (int32_t) alt;
			//printf("%1c,%11ld,%11ld,%5ld\n",id,test1,test2,test3); //DEBUG - return converted
			//printf("%1c,%11ld,%11ld,%5ld\n",id,lat,lon,alt); //DEBUG - return original

			if(id != 'A')
			{
				// Do nothing unless the message is for this Pixhawk
				continue;
			}
			// ************* Generate fake gps data from Vicon data *************
			gps_s.timestamp_position = hrt_absolute_time();
			gps_s.lat = (int32_t) lat;
			gps_s.lon = (int32_t) lon;
			gps_s.alt = (int32_t) alt;

			gps_s.timestamp_variance = hrt_absolute_time();
			gps_s.fix_type = 3;
			gps_s.eph = 0.9f;
			gps_s.epv = 1.8f;

			gps_s.timestamp_velocity = hrt_absolute_time();
			gps_s.vel_ned_valid = false;

			gps_s.timestamp_time = hrt_absolute_time();
			gps_s.time_utc_usec = hrt_absolute_time();
			gps_s.satellites_used = 10;

			// ************* Publish GPS position data *************
			if (gps_pub_fd != 0)
			{
				orb_publish( ORB_ID(vehicle_gps_position), gps_pub_fd, &gps_s );
			}
			else // On the first pass, advertise
			{
				gps_pub_fd = orb_advertise( ORB_ID(vehicle_gps_position), &gps_s);
			}

			satellite_s.count = 10;
			satellite_s.timestamp = hrt_absolute_time();
			for (int i = 0; i!=10; i++)
			{
				satellite_s.svid[i] = i+1;
				satellite_s.used[i] = 1;
			}

			// ************* Publish satellite information *************
			if (satellite_pub_fd != 0)
			{
				orb_publish( ORB_ID(satellite_info), satellite_pub_fd, &satellite_s);
			}
			else // On the first pass, advertise
			{
				 satellite_pub_fd = orb_advertise( ORB_ID(satellite_info), &satellite_s);
			}
		}
	}
	return 0;
}


