#ifndef _SPEEDTEST_LATENCY_TEST_
#define _SPEEDTEST_LATENCY_TEST_

#define LATENCY_UNITS_PER_SECOND 1000L
#define LATENCY_UNITS "ms"

long getLatency(const char *url);

/* return large positive values on error, so they sort last */
#define LATENCY_CONNECT_ERROR (1000 * LATENCY_UNITS_PER_SECOND)
#define LATENCY_DATA_ERROR (1000 * LATENCY_UNITS_PER_SECOND + 1)

#endif
