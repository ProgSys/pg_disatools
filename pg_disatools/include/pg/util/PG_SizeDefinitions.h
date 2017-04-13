#ifndef INCLUDE_PG_UTIL_PG_SIZE_DEFINITIONS_H_
#define INCLUDE_PG_UTIL_PG_SIZE_DEFINITIONS_H_

// Check GCC
#if __GNUC__
  #if __x86_64__ || __ppc64__
    #define ENV64BIT
  #else
    #define ENV32BIT
  #endif
#endif

typedef         char   i8 ;
typedef         unsigned char   ui8 ;

typedef         short   i16 ;
typedef         unsigned short   ui16 ;

typedef         int   i32 ;
typedef         unsigned int   ui32 ;

typedef         long long  i64 ;
typedef         unsigned long long   ui64 ;

#endif /* INCLUDE_PG_UTIL_PG_SIZE_DEFINITIONS_H_ */
