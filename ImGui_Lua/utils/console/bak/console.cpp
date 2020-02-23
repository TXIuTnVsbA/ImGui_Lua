#include "console.h"
#define TIME_SIZE 10
//#define TIME_FORMAT "%I:%M:%S"
#define TIME_FORMAT "%T"
c_console g_console{};

std::string get_time_string()
{
	//Time related variables
	time_t current_time;
	struct tm* time_info;
	static char timeString[TIME_SIZE];

	//Get current time
	time(&current_time);
	time_info = localtime(&current_time);

	//Get current time as string
	strftime(timeString, sizeof(timeString), TIME_FORMAT, time_info);
	return timeString;
}

bool c_console::allocate(const char* window_name) {
	//if (!AllocConsole()) {
		//_RPTF1(_CRT_ERROR, "Failed to allocate console. Error code: %i", GetLastError());
		//return false;
	//}

	allocate();

	if (!SetConsoleTitleA(window_name)) {
		_RPTF1(_CRT_WARN, "Failed to set console title. Error code: %i", GetLastError());
		return false;
	}
	return true;
}

bool c_console::allocate() {
#ifdef _DEBUG
	/*
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	freopen("CONERR$", "w", stderr);
	*/
	_iobuf* out_data;
	_iobuf* in_data;
	_iobuf* err_data;

	const errno_t out_res = freopen_s(&out_data, "CONOUT$", "w", stdout);
	const errno_t in_res = freopen_s(&in_data, "CONIN$", "r", stdin);
	const errno_t err_res = freopen_s(&err_data, "CONOUT$", "w", stderr);

	if (out_res != 0) {
		_RPTF1(_CRT_ERROR, "Failed to open stdout filestream. Error code: %i", out_res);
		return false;
	}

	if (in_res != 0) {
		_RPTF1(_CRT_ERROR, "Failed to open stdin filestream. Error code: %i", in_res);
		return false;
	}

	if (err_res != 0) {
		_RPTF1(_CRT_ERROR, "Failed to open stderr filestream. Error code: %i", err_res);
		return false;
	}
	
#else
	setbuf(stdout, out_buf);
#endif
	return true;
}

void c_console::detach() {
	fclose((FILE*)stdin);
	fclose((FILE*)stdout);
	fclose((FILE*)stderr);
	FreeConsole();
}

void c_console::set_console_color(WORD color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void c_console::log(const char* fmt, ...) {
	if (!fmt) return; //if the passed string is null return
	if (strlen(fmt) < 2) return;

	//Set up va_list and buffer to hold the params 
	va_list va_alist;
	char logBuf[256] = { 0 };

	//Do sprintf with the parameters
	va_start(va_alist, fmt);
	_vsnprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);

	//Output to console
	if (logBuf[0] != '\0')
	{
		set_console_color(FOREGROUND_INTENSE_GREEN);
		std::printf("[%s]", get_time_string().c_str());
		//std::printf("[%I64d]", GetTickCount64());
		set_console_color(FOREGROUND_WHITE);
		std::printf(": %s\n", logBuf);
	}

	if (FileLog)
	{
		logFile << logBuf << std::endl;
	}
}

void c_console::enable_log_file(std::string filename) {
	logFile.open(filename.c_str());
	if (logFile.is_open())
		FileLog = true;
}

