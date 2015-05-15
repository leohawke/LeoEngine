g++ -c -std=c++11 src/* -DMULTI_THREAD=1 -DPLATFORM_WIN32=1 -masm=intel -msse2 -I ./Include/
//emmintrin.h is for SSE2, so it should probably be -msse2
ar rcs Indepaltform.a *.o