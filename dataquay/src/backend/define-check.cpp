
#ifdef USE_REDLAND
#ifdef USE_SORD
#error Only one of USE_REDLAND and USE_SORD may be defined
#endif
#endif

#ifndef USE_REDLAND
#ifndef USE_SORD
#error One of USE_REDLAND or USE_SORD must be defined
#endif
#endif

