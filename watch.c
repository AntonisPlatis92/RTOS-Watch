#include <sys/prex.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char stack[3][1024];
int watch_hours,watch_min,watch_sec;
int mode=0;
int stopwatch_running=0,stopwatch_pause=0;
int stopwatch_ekatosta=0,stopwatch_sec=0,stopwatch_min=0,stopwatch_hours=0;

u_long get_time() {  //Get current time
        device_t rtc_dev;
	u_long sec;
	device_open("rtc", 0, &rtc_dev);
  	device_ioctl(rtc_dev, RTCIOC_GET_TIME, &sec);
	device_close(rtc_dev);
	return sec;
}

static thread_t thread_run(void (*start)(void), char *stack){ //Running a thread
	thread_t t;

	if (thread_create(task_self(), &t) != 0)
		return 0;
	if (thread_load(t, start, stack) != 0)
		return 0;
	if (thread_resume(t) != 0)
		return 0;
	return t;
}

static void count_sec(){   //Thread for counting seconds. Sleeps for 1 sec, then wakes up, increments. 
       thread_yield();
       while (1){
       	  timer_sleep(1000,NULL);
          watch_sec=watch_sec+1;
          if (watch_sec==60){
              watch_sec=0;
              watch_min=watch_min+1;
              if (watch_min==60){
                  watch_min=0;
                  watch_hours=(watch_hours+1)%24;   
              }
          }
        }
        thread_terminate(thread_self());
}
static void get_char(){  //Thread for getting the char hit from keyboard
     
     char button;
     thread_yield();
     while (1){
        button=getchar();
        if (mode==0){  //Depeding the mode do the proper changes (0='watch' 1='stopwatch')
            if (button=='t') {
                mode=1;
                printf("\33[2J");
                printf("Stopwatch Mode:\n");
                printf("%02d:%02d:%02d:%02d\n",stopwatch_hours,stopwatch_min,stopwatch_sec,stopwatch_ekatosta);
                printf("t:Toggle Mode   r:Reset   s:Start/Stop   p:Pause\n");
            }
            if (button=='h') watch_hours=(watch_hours+1)%24;
            if (button=='m') {
                watch_min=(watch_min+1)%60;
                if (watch_min==0) watch_hours=(watch_hours+1)%24;
            }
            if (button=='z') watch_sec=0;
        }
        else if (mode==1){
            if (button=='t') {
               mode=0;
            }   
            if (button=='r') {
               stopwatch_hours=0;
               stopwatch_min=0;
               stopwatch_sec=0;
               stopwatch_ekatosta=0;
            }
            if (button=='s') {
                if (stopwatch_running==0) stopwatch_running=1;
                else if (stopwatch_running==1) stopwatch_running=0;
            }
            if (button=='p'){
                if (stopwatch_pause==0) stopwatch_pause=1;
                else if (stopwatch_pause==1) stopwatch_pause=0;
            }
        }
     }
     thread_terminate(thread_self());
}


int main(int argc,char *argv[]){
   
   printf("Hello RTOS!\nInitialing...\n"); //Main function
   timer_sleep(2000,NULL);
   
   int years,days,hours; //Calculate the current Time of Day in localtime
   u_long sec=get_time();
   years=sec/(3600*24*365);
   sec=sec-3600*24*365*years;
   days=sec/(3600*24);
   sec=sec-3600*24*days;
   watch_min=(sec/60)%60;
   hours=sec/3600;
   watch_hours=(hours)%24;
   watch_sec=sec%60;
   
   
   
   if (thread_run(get_char, stack[0]+1024) == 0)	panic("failed to create thread"); //Create 2 threads, 1 for counting seconds, 1 for waiting for key to be pressed
   if (thread_run(count_sec, stack[1]+1024) == 0)	panic("failed to create thread");

   while (1) {
       if (mode==0){ //Watch Mode
               printf("\33[2J"); //Clear screen
               printf("Watch Mode:\nTime = %02d:%02d:%02d\n",watch_hours,watch_min,watch_sec);  //Print the current time
               printf("t:Toggle Mode   h:Hours+1   m:Minutes+1   z:Reset Seconds\n");
               
               timer_sleep(500,NULL);
               
               
         }
         else if (mode==1){ //Stopwatch mode
            if (stopwatch_running==1){
                stopwatch_ekatosta=stopwatch_ekatosta+1; 
                if (stopwatch_ekatosta==100){
                    stopwatch_ekatosta=0;
                    stopwatch_sec=stopwatch_sec+1;
                    if (stopwatch_sec==60){
                         stopwatch_sec=0;
                         stopwatch_min=stopwatch_min+1;
                       	 if (stopwatch_min==60){
                               stopwatch_min=0;
                               stopwatch_hours=stopwatch_hours+1;
                         }
                     }
                }
            }
            if (stopwatch_pause==0){
                printf("\33[2J");
                printf("Stopwatch Mode:\n");
                printf("%02d:%02d:%02d:%02d\n",stopwatch_hours,stopwatch_min,stopwatch_sec,stopwatch_ekatosta);
                printf("t:Toggle Mode   r:Reset   s:Start/Stop   p:Pause\n");
            }    
            timer_sleep(10,NULL); //Delay in order to see the stopwatch clock changes for natural in screen
         }
   }
}
