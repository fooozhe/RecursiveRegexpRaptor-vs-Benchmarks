#include <stdio.h>

#include "regexp4/regexp4.h"
#include "main.h"

void regexp4_find_all( char *pattern, char *subject, int subject_len, int repeat ){
  TIME_TYPE start, end;
  int found, time, best_time = 0;

  do{
    found = 0;

    GET_TIME( start );

    found = regexp4( subject, pattern, subject_len );

    GET_TIME( end );
    time = TIME_DIFF_IN_MS( start, end );
    if( !best_time || time < best_time ) best_time = time;
  } while( --repeat > 0 );

  // printf("regexp4 log start\n" );

  // int i = 0, max = totCatch_4();
  // while( ++i <= max )
  //   printf( "%.*s %ld %ld\n", (int)lenCatch_4( i ), gpsCatch_4( i ), gpsCatch_4(i) - subject, (gpsCatch_4(i) - subject) + lenCatch_4(i));

  // printf("regexp4 log end\n" );

  printResult( "regexp4", best_time, found );
}
