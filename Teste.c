#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct timeval ini_Time;

struct timeval up_Time;

int main()
{
  gettimeofday(&ini_Time, NULL);
  double elapse = 0;
  do
  {
    gettimeofday(&up_Time, NULL);

    elapse = (up_Time.tv_sec - ini_Time.tv_sec) + (up_Time.tv_sec - ini_Time.tv_sec) / 10000;
    printf("%f\n", elapse);
  } while (elapse < 10);

  return 0;
}
