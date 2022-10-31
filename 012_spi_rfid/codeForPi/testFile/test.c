#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256           ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH]; ///< The receive buffer from the LKM

int main()
{
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   printf("Starting device test code example...\n");
   fd = open("/dev/rfid_rc522_dev", O_RDWR); // Open the device with read/write access
   if (fd < 0)
   {
      perror("Failed to open the device...");
      return errno;
   }

   printf("Press ENTER to read back from the device...\n");
   getchar();

   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH); // Read the response from the LKM
   if (ret < 0)
   {
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%2.2X %2.2X %2.2X %2.2X]\n", receive[0], receive[1], receive[2], receive[3]);
   printf("End of the program\n");
   return 0;
}
