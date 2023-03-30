#include <iostream>
#include <unistd.h>
#include <sys/sem.h>
#include <cstdlib>
#include "AirportAnimator.hpp"

using namespace std;

AirportAnimator planes;

int passangers; //holds the number of passengers in the airport
int tours;//hold the number of tours
int tourscount;// Will count the numbers of tours each time
pthread_t threads[8]; //Create a array of 8 threads
int sem_id; //runaway semaphore
int sem_id2;//passengers semaphore


void *myFunc(void *param)
{
	int *individual = (int *) param; // change "generic" pointer to point to type we intend
	
	while(tourscount < tours) //while the number of tours completed is less than the total number of tours
	{
		 planes.updateStatus(*individual, "BOARD");
		//Load 12 passengers in the plane
		for(int passenger=1; passenger<=12; passenger++)
	    	{
	    		//reduce the sem op of the passengers semaphore which is equal to the number of total passengers.
	    		struct sembuf semWaitCommand[1];

		  	semWaitCommand[0].sem_num = 0;
		  	semWaitCommand[0].sem_op = -1; 
		  	semWaitCommand[0].sem_flg = 0;
		  
		  	semop(sem_id2, semWaitCommand, 1);
		  	
		  	//upadate the number of passangers each time
			planes.updatePassengers(*individual, passenger);
			sleep(rand()%3); //random delay for passengers
	    	}
	    	  //Once the plane is full, taxi to the runaway
		  planes.updateStatus(*individual, "TAXI");
		  planes.taxiOut(*individual);
		  
		  //Wait for the runaway to be empty
		  struct sembuf semWaitCommand[1];

		  semWaitCommand[0].sem_num = 0;
		  semWaitCommand[0].sem_op = -1; 
		  semWaitCommand[0].sem_flg = 0;
		  
		  semop(sem_id, semWaitCommand, 1);
		  
		  //Make the animation smoother
		  sleep(1);
		  
		  //take off animation and change th status
		  planes.updateStatus(*individual, "TKOFF");
		  planes.takeoff(*individual);
		  
		  //Change status to on tour
		  planes.updateStatus(*individual,"TOUR");
		  
		  //Signal that the plane is fliying and other plane can take the runaway
		  struct sembuf semSignalCommand[1];

		  semSignalCommand[0].sem_num = 0;
		  semSignalCommand[0].sem_op = +1; 
		  semSignalCommand[0].sem_flg = 0;

		  semop(sem_id, semSignalCommand, 1);
		  
		  //stay on tour for a random period of time between 15 and 45
		  sleep(15+rand()%31);
		  
		  //request the landing
		  planes.updateStatus(*individual,"LNDRQ");

		  //wait to land until the runaway is empty
		  struct sembuf semWaitCommand2[1];

		  semWaitCommand2[0].sem_num = 0;
		  semWaitCommand2[0].sem_op = -1; 
		  semWaitCommand2[0].sem_flg = 0;
		  
		  semop(sem_id, semWaitCommand2, 1);
		  
		  //make the animation smoother
		  sleep(2);
	
	 	  //start the landing animation and update the status to "LAND"
		  planes.updateStatus(*individual,"LAND");
		  planes.land(*individual);

		  planes.updateStatus(*individual, "TAXI");
		  
		  //signal that the runaway is free to use
		  struct sembuf semSignalCommand2[1];

		  semSignalCommand2[0].sem_num = 0;
		  semSignalCommand2[0].sem_op = +1; 
		  semSignalCommand2[0].sem_flg = 0;
		     
		  semop(sem_id, semSignalCommand2, 1);
		  
		  //taxi back to the terminal
		  planes.taxiIn(*individual);

		  planes.updateStatus(*individual, "DEPLN");
		  
		  //Leave the passengers back at the terminal
		  for(int passenger=11; passenger>=0; passenger--)
		    {
		    	//Singal that the passengers are back a the terminal to the second semaphore
		      	struct sembuf semSignalCommand[1];

		  	semSignalCommand[0].sem_num = 0;
		  	semSignalCommand[0].sem_op = +1; 
		  	semSignalCommand[0].sem_flg = 0;
		     
		  	semop(sem_id2, semSignalCommand, 1);
		  	
		  	//update the number of passengers
		      	planes.updatePassengers(*individual, passenger);
		      	//delay deploying the passengers
		      	sleep(1);
		    }
		  //update the current number of tours
		  tourscount ++;
		  //reflect this update in the screen
		  planes.updateTours(tourscount);
		  sleep(2);
	}
	//when the total number of tours is accomplished and the plane has deployed all the passengers display done
	planes.updateStatus(*individual, "DONE");
	//Make animation look smoother
	sleep(1);
	return NULL;
}
   
int main(int argc, char *argv[])
{
	passangers = atoi(argv[1]);//take the first parameter as the passengers
	tours = atoi(argv[2]);//take the second parameter a the passengers
	
	sem_id = semget(10, 1, IPC_CREAT|IPC_EXCL|0600); //create the runaway semaphore
	
	 if (sem_id <0) // semaphore creation failure - likely already exists !
	 {
	      sem_id = semget(10, 1, 0);
	 }
	
	semctl(sem_id, 0, SETVAL, 1); //inicializate the value to one
	
	
	
	sem_id2 = semget(101, 1, IPC_CREAT|IPC_EXCL|0600);//create the passengers semaphore
	
	 if (sem_id2 <0) // semaphore creation failure - likely already exists !
	 {
	      sem_id2 = semget(101, 1, 0);
	 }
	
	semctl(sem_id2, 0, SETVAL, passangers);//inicializate the value to the number of total passengers
	
	planes.init();//inicializate the airport 
	
	
	//inicializate all the threads
	int id0=0;
	pthread_create(&threads[0], NULL, myFunc, &id0);
	
	int id1=1;
	pthread_create(&threads[1], NULL, myFunc, &id1);
	
	int id2=2;
	pthread_create(&threads[2], NULL, myFunc, &id2);
	
	int id3=3;
	pthread_create(&threads[3], NULL, myFunc, &id3);
	
	int id4=4;
	pthread_create(&threads[4], NULL, myFunc, &id4);
	
	int id5=5;
	pthread_create(&threads[5], NULL, myFunc, &id5);
	
	int id6=6;
	pthread_create(&threads[6], NULL, myFunc, &id6);
	
	int id7=7;
	pthread_create(&threads[7], NULL, myFunc, &id7);
	
	//wait for all the threads to finish
        pthread_join(threads[0], NULL);
        
        pthread_join(threads[1], NULL);
        
        pthread_join(threads[2], NULL);
        
        pthread_join(threads[3], NULL);
        
        pthread_join(threads[4], NULL);
        
        pthread_join(threads[5], NULL);
        
        pthread_join(threads[6], NULL);
        
        pthread_join(threads[7], NULL);
        
        //finish the airport display and animations
        planes.end();
        
        //return on succes.
        return 0;	
  	
}


