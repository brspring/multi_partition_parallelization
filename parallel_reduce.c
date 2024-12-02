#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>


#define DEBUG 0
#define MAX_THREADS 4
#define LOOP_COUNT 1
#define FLOAT 1
#define DOUBLE 2

#define TYPE FLOAT

#if TYPE == FLOAT
   #define element_TYPE float
#elif TYPE == DOUBLE   
   #define element_TYPE double
#endif   

#define NTIMES 10

#if TYPE == FLOAT
   #define MAX_TOTAL_ELEMENTS (500*1000*1000) 
                                           
                                            
#elif TYPE == DOUBLE   
   #define MAX_TOTAL_ELEMENTS (250*1000*1000) 
                                          
                                        
#endif   


pthread_t parallelReduce_Thread[ MAX_THREADS ];
int parallelReduce_thread_id[ MAX_THREADS ];
element_TYPE parallelReduce_partialSum[ MAX_THREADS ];   

int parallelReduce_nThreads; 
int parallelReduce_nTotalElements; 
element_TYPE InputVector[ MAX_TOTAL_ELEMENTS ];                           
element_TYPE *InVec = InputVector;
pthread_barrier_t parallelReduce_barrier;

int min( int a, int b )
{
   if( a < b )
      return a;
   else
      return b;
}

element_TYPE plus( element_TYPE a, element_TYPE b )
{
    return a + b;
}

void *reducePartialSum(void *ptr)
{
    int myIndex = *((int *)ptr);
    //int nElements = parallelReduce_nTotalElements / parallelReduce_nThreads;
    int nElements = (parallelReduce_nTotalElements+(parallelReduce_nThreads-1))
                    / parallelReduce_nThreads;
        
    // assume que temos pelo menos 1 elemento por thhread
    int first = myIndex * nElements;
    int last = min( (myIndex+1) * nElements, parallelReduce_nTotalElements ) - 1;
    
    while( true ) {
        pthread_barrier_wait( &parallelReduce_barrier );    
        register element_TYPE myPartialSum = 0;
        for( int i=first; i<=last ; i++ )
           myPartialSum += InVec[i];

        parallelReduce_partialSum[ myIndex ] = myPartialSum;     
        
        pthread_barrier_wait( &parallelReduce_barrier );    
        if( myIndex == 0 )
            return NULL;
          
    }
    if( myIndex != 0 )
        pthread_exit( NULL );
          
    return NULL;      
}


element_TYPE parallel_reduceSum( element_TYPE InputVec[], 
                                 int nTotalElements, int nThreads )
{

    static int initialized = 0;
    parallelReduce_nTotalElements = nTotalElements;
    parallelReduce_nThreads = nThreads;
    
    InVec = InputVec;
    
    if( ! initialized ) { 
       pthread_barrier_init( &parallelReduce_barrier, NULL, nThreads );
       // cria todas as outra threds trabalhadoras
       parallelReduce_thread_id[0] = 0;
       for( int i=1; i < nThreads; i++ ) {
         parallelReduce_thread_id[i] = i;
         pthread_create( &parallelReduce_Thread[i], NULL, 
                      reducePartialSum, &parallelReduce_thread_id[i]);
       }

       initialized = 1;
    }

    reducePartialSum( &parallelReduce_thread_id[0] ); 
    element_TYPE globalSum = 0;
    for( int i=0; i<nThreads ; i++ ) {
        //printf( "globalSum = %f\n", globalSum );
        globalSum += parallelReduce_partialSum[i];
    } 
    return globalSum;
}

int main( int argc, char *argv[] )
{
    int i;
    int nThreads;
    int nTotalElements;
    
    chronometer_t parallelReductionTime;
    
    if( argc != 3 ) {
         printf( "usage: %s <nTotalElements> <nThreads>\n" ,
                 argv[0] ); 
         return 0;
    } else {
         nThreads = atoi( argv[2] );
         if( nThreads == 0 ) {
              printf( "usage: %s <nTotalElements> <nThreads>\n" ,
                 argv[0] );
              printf( "<nThreads> can't be 0\n" );
              return 0;
         }     
         if( nThreads > MAX_THREADS ) {  
              printf( "usage: %s <nTotalElements> <nThreads>\n" ,
                 argv[0] );
              printf( "<nThreads> must be less than %d\n", MAX_THREADS );
              return 0;
         }     
         nTotalElements = atoi( argv[1] ); 
         if( nTotalElements > MAX_TOTAL_ELEMENTS ) {  
              printf( "usage: %s <nTotalElements> <nThreads>\n" ,
                 argv[0] );
              printf( "<nTotalElements> must be up to %d\n", MAX_TOTAL_ELEMENTS );
              return 0;
         }     
    }
    
    
    #if TYPE == FLOAT
        printf( "will use %d threads to reduce %d total FLOAT elements\n\n", nThreads, nTotalElements );
    #elif TYPE == DOUBLE   
        printf( "will use %d threads to reduce %d total DOUBLE elements\n\n", nThreads, nTotalElements );
    #endif   
    
    for( int i=0; i<MAX_TOTAL_ELEMENTS ; i++ )
        InputVector[i] = (element_TYPE)1;
         
      element_TYPE globalSum;
      int start_position = 0;
      InVec = &InputVector[start_position];

    for( int i=0; i<NTIMES ; i++ ) {
        globalSum = parallel_reduceSum( InVec,
                                        nTotalElements, nThreads );
        start_position += nTotalElements;
        if( (start_position + nTotalElements) > MAX_TOTAL_ELEMENTS )
            start_position = 0;
        InVec = &InputVector[start_position];                            
    }                                           
    return 0;
}