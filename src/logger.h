#ifndef LOGGER_H_
#define LOGGER_H_

void info_log(const char *fmt, ...);
void warning_log(const char *fmt, ...);
void error_log(const char *fmt, ...);
void debug_log(const char *fmt, ...);

#endif	// LOGGER_H_
