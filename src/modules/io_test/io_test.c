/*
 * io_test.c
 *
 *  Created on: Nov 10, 2014
 *      Author: svankoot
 */
#include <nuttx/config.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <uORB/uORB.h>
#include <uORB/topics/actuator_armed.h>
#include <uORB/topics/manual_control_setpoint.h>
#include <uORB/topics/test_motor.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/topics/vehicle_vicon_position.h>
#include <uORB/topics/satellite_info.h>
#include <drivers/drv_hrt.h>

#define IS_ECHO 0

__EXPORT int io_test_main(int argc, char *argv[]);

int io_test_main(int argc, char *argv[])
{
	/*
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
	*/

	// Advertise/Publish to gps position topic
	struct vehicle_gps_position_s gps_s;
	memset(&gps_s, 0, sizeof(gps_s));
	//int gps_pub_fd = 0;

	// Advertise/Publish to satellite info topic
	struct satellite_info_s satellite_s;
	memset(&satellite_s, 0, sizeof(satellite_s));
	//int satellite_pub_fd = 0;

	// Variables to store GPS coordinates
	long int lat;
	long int lon;
	long int alt;

	// Publish Vicon Position directly
	struct vehicle_vicon_position_s vicon_position;
	memset(&vicon_position, 0, sizeof(vicon_position));
	int vicon_pub_fd = 0;

	// Variables to store Vicon data
	int scanCheck = 0;				// Validate incoming data
	float x = 0;
	float y = 0;
	float z = 0;
	float yaw = 0;						// Positions
	float prevX = 0;
	float prevY = 0;
	float prevZ = 0;
	float vx,vy,vz; 						// Velocities
	char id;
	char buffer[80];

	// Variables for time keeping
	uint64_t prevT = 0;
	uint64_t curT = 0;		// units: microseconds
	float dT = 0;					// units: seconds
	bool firstTime = true;

	// DEBUG Velocity calculations (make sure have good Hz)
	uint64_t lastCheck = hrt_absolute_time();
	uint64_t thisCheck = hrt_absolute_time();
	int loopCount = 0;
	int xCount = 0;
	int yCount = 0;
	int zCount = 0;

	// Main Loop (Read from serial 5 and publish GPS/Vicon data)
	while (1)
	{
		// Read from serial 5 port (get Vicon GPS info through XBee)
		gets(buffer);

		// DEBUG
		thisCheck = hrt_absolute_time();
		if((thisCheck - lastCheck) > 5000000) // update every 5 seconds
		{
			lastCheck = thisCheck;
			printf("Valid data points received in last 5 seconds: %d\n(X,Y,Z): (%d,%d,%d)\n",
					loopCount,xCount,yCount,zCount);
			loopCount = 0;
			xCount = 0;
			yCount = 0;
			zCount = 0;
		}

		// ************* Check for initialization message: 'BEGIN' *************
		if(strcmp(buffer,"BEGIN") == 0)
		{
			//printf("%s\n",buffer);  // Establish 1st communication
			continue;
		}
		else if(strcmp(buffer,"NODATA") == 0)
		{
			//printf("NO DATA!\n");
			continue;				 // Ignore packets with no useful data
		}
		else
		{
			// Parse Vicon/GPS data from XBee into variables
			scanCheck = sscanf(buffer,"%c,%ld,%ld,%ld,%f,%f,%f",
						  &id,&lat,&lon,&alt,&x,&y,&yaw); // Read gps/vicon coordinates

			//DEBUG - return original
			//printf("%1c,%11ld,%11ld,%5ld,%6.3f,%6.3f,%6.3f,%6.3f\n",
			//		id,lat,lon,alt,(double)x,(double)y,(double)z,(double)yaw);

			//printf("%1c,%11ld,%11ld,%5ld\n",id,lat,lon,alt); //DEBUG - return original
			if(scanCheck < 7)
			{
				printf("sscanf failed on \"%s\"! (returned: %d)\n",buffer,scanCheck);
				continue;	// Don't use bad data!
			}
			if(id != 'A')
			{
				// Do nothing unless the message is for this Pixhawk
				continue;
			}

			//DEBUG
			loopCount++;	// Count how many valid data points received

			//DEBUG - Validate input from sscanf
			float maxPos = 7000;	// will never be more than 7 meters from center
			float maxAlt = 5000;	// will never be more than 5 meters above ground
			if(x < -maxPos || x > maxPos)
			{
				printf("X OUT OF RANGE: %.4f\n",(double)x);
				continue;
			}
			if(y < -maxPos || y > maxPos)
			{
				printf("Y OUT OF RANGE: %.4f\n",(double)y);
				continue;
			}
			if(alt < -maxAlt || alt > maxAlt)
			{
				printf("ALT OUT OF RANGE: %.4f\n",(double)alt);
				continue;
			}

			// Convert position from mm to meters
			x /= 1000.0f;
			y /= 1000.0f; 	// Y already inverted in sender code
			z = (float) alt;
			z /= -1000.0f; // NED frame, z is down

			curT = hrt_absolute_time(); // Get current timestamp

			// Can't calculate velocity on first pass
			if(!firstTime)
			{
				dT = (curT - prevT) / 1000000.0f;
				if(dT < .000001f)
				{
					// Something went wrong... prevent divide by 0
					printf("ERROR: dT == 0!!!\n"); // DEBUG
					vx = 0;
					vy = 0;
					vz = 0;
				}
				else
				{
					// Calculate velocity with a backward difference approximation
					vx = (x - prevX) / dT;
					vy = (y - prevY) / dT;
					vz = (z - prevZ) / dT;
				}
				//DEBUG - Count how many times threshold velocity is exceeded in each direction
				float tv = 1; // threshold velocity
				if(vx > tv || vx < -1*tv)
				{
					xCount++;
					//printf("X velocity: %.3f\n",(double)vx);
					//printf("x: %.4f\tprevX: %.4f\ndT: %.7f\n",(double)x,(double)prevX,(double)dT);
				}
				if(vy > tv || vy < -1*tv)
				{
					yCount++;
					//printf("Y velocity: %.3f\n",(double)vy);
					//printf("y: %.4f\tprevY: %.4f\ndT: %.7f\n",(double)y,(double)prevY,(double)dT);
				}
				if(vz > tv || vz < -1*tv)
				{
					zCount++;
					//printf("Z velocity: %.3f\n",(double)vz);
					//printf("z: %.4f\tprevZ: %.4f\ndT: %.7f\n",(double)z,(double)prevZ,(double)dT);
				}
			}
			else
			{
				// Initialize variables on first pass through
				firstTime = false;
				prevT = curT;
				vx = 0;
				vy = 0;
				vz = 0;
			}

			// Set previous variables for next iteration
			prevT = curT;
			prevX = x;
			prevY = y;
			prevZ = z;

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

			// ************* Generate Vicon position topic data *************
			vicon_position.timestamp = hrt_absolute_time();
			vicon_position.xy_valid	 = true;
			vicon_position.z_valid	 = true;
			vicon_position.v_xy_valid	 = true;
			vicon_position.v_z_valid	 = true;
			vicon_position.x = x;
			vicon_position.y = y;
			vicon_position.z = z;
			vicon_position.vx = vx;
			vicon_position.vy = vy;
			vicon_position.vz = vz;
			vicon_position.yaw = yaw;
			vicon_position.eph = 0.9f;
			vicon_position.epv = 1.8f;
			//vicon_position.xy_global = false;
			//vicon_position.z_global = false;


			// Publish/Advertise Vicon Position
			if (vicon_pub_fd != 0)
			{
				orb_publish( ORB_ID(vehicle_vicon_position), vicon_pub_fd, &vicon_position);
			}
			else // On the first pass, advertise
			{
				vicon_pub_fd = orb_advertise( ORB_ID(vehicle_vicon_position), &vicon_position);
			}
/*
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
			*/
		}
	} // end while(1)


	return 0;
}


